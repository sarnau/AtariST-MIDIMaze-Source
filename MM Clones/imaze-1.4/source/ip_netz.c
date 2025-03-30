/*
** - - -  iMaze  - - -
**
** Copyright (c) 1993-2001 by Hans-Ulrich Kiel & Joerg Czeranski
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the authors may not be used to endorse or promote
**    products derived from this software without specific prior written
**    permission.
** 4. The name ``iMaze'' may not be used for products derived from this
**    software unless a prefix or a suffix is added to the name.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
** INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
** STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
** IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
**
** Datei: ip_netz.c
**
** Kommentar:
**  Netzwerkkommunikation ueber TCP/IP
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <netdb.h>

#include "argv.h"
#include "fehler.h"
#include "speicher.h"
#include "netzwerk.h"
#include "system.h"

static char sccsid[] = "@(#)ip_netz.c	3.11 12/3/01";


/* Default-TCP-Port des Servers */
#ifndef DEFAULT_PORT
#define DEFAULT_PORT /* phone +49-*/5323/*-72-3896 */
#endif

/* Default-Hostname des Servers */
#ifndef DEFAULT_SERVER
#define DEFAULT_SERVER "imaze.rz.tu-clausthal.de"
#endif

#ifndef ADDRLEN_T
#define ADDRLEN_T int
#endif

/* maximale Groesse, mit der Spiel-Pakete gesendet und empfangen werden */
#define MAX_SPIEL_PAKET 1024


static char *set_server_name = NULL;
struct arg_option net_opts[] =
{
	{ Arg_String, "H", &set_server_name, "set server", "host[:port]" },
	{ Arg_End }
};


/* Systemvariable: letzter aufgetretener Fehlercode oder 0 */
extern int errno;


static int tcp_deskriptor = -1;     /* Deskriptor fuer TCP-Verbindung */
static int udp_deskriptor = -1;     /* Deskriptor fuer UDP-Socket */
static int letzte_paketnummer = -1; /* Nummer des letzten empfangenen
                                       Spiel-Paketes */


/*
** netzwerk_fehler
**  ruft uebler_fehler mit einem Standard-Fehlertext auf
**
** Parameter:
**  ursache: Textzeile, die die Fehlerursache beschreibt
*/
static void netzwerk_fehler(char *ursache)
{
	static char *meldung[] = { "iMaze - Network Error", "",
		NULL, NULL, NULL }; /* Fehlermeldung */

	meldung[2] = ursache;       /* Fehlerursache */
	meldung[3] = fehler_text(); /* Standard-Fehlertext */

	/* Fehlermeldung ausgeben */
	uebler_fehler(meldung, NULL);
}


/*
** milder_netzwerk_fehler
**  ruft milder_fehler mit einem Standard-Fehlertext auf
**  und beendet danach die Verbindung
**
** Parameter:
**  ursache: Textzeile, die die Fehlerursache beschreibt
*/
static void milder_netzwerk_fehler(char *ursache)
{
	static char *meldung[] = { "iMaze - Network Error", "",
		NULL, NULL, NULL }; /* Fehlermeldung */

	meldung[2] = ursache;       /* Fehlerursache */
	meldung[3] = fehler_text(); /* Standard-Fehlertext */

	/* Fehlermeldung ausgeben */
	milder_fehler(meldung, NULL);

	verbindung_beenden();
}


/*
** tcp_deskriptor_pruefen
**  prueft, ob bereits eine TCP-Verbindung aufgebaut ist; andernfalls
**  wird die Fehlerroutine aufgerufen
*/
static void tcp_deskriptor_pruefen(void)
{
	/* noch keine TCP-Verbindung aufgebaut? */
	if (tcp_deskriptor < 0)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Not yet connected.", NULL }; /* Fehlermeldung */

		uebler_fehler(meldung, NULL);
	}
}


/*
** udp_deskriptor_pruefen
**  prueft, ob bereits ein UDP-Socket geoeffnet ist; andernfalls
**  wird die Fehlerroutine aufgerufen
*/
static void udp_deskriptor_pruefen(void)
{
	/* noch kein UDP-Socket geoeffnet? */
	if (udp_deskriptor < 0)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Not yet in game.", NULL }; /* Fehlermeldung */

		uebler_fehler(meldung, NULL);
	}
}


/*
** paket_handler
**  gibt saemtliche bereits angekommenen Pakete an den uebergeordneten
**  Netzwerkteil weiter
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  letzte_paketnummer wird erhoeht
*/
static int paket_handler(void)
{
	/* Pakete auswerten solange vorhanden; Abbruch bei Fehler EWOULDBLOCK */
	for (;;)
	{
		int paket_laenge;               /* Groesse des aktuellen Pakets */
		int paketnummer;                /* Nummer des aktuellen Pakets */
		u_char puffer[MAX_SPIEL_PAKET]; /* Puffer fuer ein Paket */

		/* ein Paket empfangen */
		if ((paket_laenge = recv(udp_deskriptor, (void *)puffer,
			sizeof puffer, 0)) <= 0)
		{
			/* Fehler EWOULDBLOCK bedeutet, dass keine Pakete mehr
			   bereitstehen; Schleife verlassen */
			if (errno == EWOULDBLOCK || errno == EAGAIN ||
				paket_laenge == 0)
				break;

			/* Fehler EINTR (Interrupted System Call) ignorieren */
			if (errno == EINTR)
				continue;

			/* bei anderem Fehler Fehlerroutine aufrufen */
			milder_netzwerk_fehler("Can't receive game packet:");
			return 1;
		}

		/* falls Paket nicht einmal die Nummer enthaelt oder Puffer
		   komplett voll geworden und moeglicherweise uebergelaufen ist,
		   Paket ignorieren */
		if (paket_laenge < 2 || paket_laenge == sizeof puffer)
			continue;

		/* Nummer des Paketes aus dem Paket auslesen */
		paketnummer = (u_short)puffer[0] * 256 | (u_short)puffer[1];

		/* falls schon die Nummer eines vorigen Paketes bekannt ist,
		   ueberpruefen, dass die aktuelle Nummer "modulo 65536" groesser
		   ist; sonst Paket ignorieren */
		if (letzte_paketnummer >= 0 &&
			(unsigned short)(letzte_paketnummer - paketnummer) < 32768)
			continue;

		/* Nummer merken, um spaeter die Reihenfolge pruefen zu koennen */
		letzte_paketnummer = paketnummer;

		/* hier koennte das Paket kopiert werden, falls notwendig */

		/* Routine im uebergeordneten Netzwerkteil aufrufen, die den
		   Paketinhalt auswertet; die zwei Bytes, die die Nummer
		   enthalten, abschneiden */
		if (spiel_paket_angekommen(paket_laenge - 2, puffer + 2))
			return 1;
	}

	/* kein Fehler */
	return 0;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** netzwerk_init
**  Dummy-Routine, die den Netzwerkteil initialisiert
**
** Rueckgabewert:
**  0 bei ordentlicher Initialisierung, 1 bei Fehler
*/
int netzwerk_init(void)
{
	/* Zur Zeit passiert nichts, also treten keine Fehler auf */
	return 0;
}


char *net_get_server_name(int *override)
{
	if (override != NULL)
		*override = set_server_name != NULL;

	if (set_server_name != NULL)
		return set_server_name;
	else
		return DEFAULT_SERVER;
}


/*
** verbindung_aufbauen
**  baut die Verbindung zum Server auf und schaltet sie in den
**  Prolog-Modus; hier: baut TCP-Verbindung auf
**
** Parameter:
**  server_name: Name des Hosts, auf dem der Server laeuft, oder NULL fuer
**               den Default-Host; hier in der Form [hostname/ip][:[port]];
**               falls hostname nicht angegeben, dann localhost
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  setzt tcp_deskriptor
*/
int verbindung_aufbauen(char *server_name)
{
	char *server_hostname; /* Hostname/IP-Nummer des Servers */
	char *server_portname; /* Name/Nummer des TCP-Ports */
	int hostname_kopiert;  /* Flag, ob Hostname in extra Speicher
	                          kopiert wurde */
	struct sockaddr_in server_tcp_adresse; /* komplette Adresse des
	                                          Servers */

	/* bereits eine TCP-Verbindung aufgebaut? */
	if (tcp_deskriptor >= 0)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Already connected.", NULL }; /* Fehlermeldung */

		uebler_fehler(meldung, NULL);
	}

	/* bereits ein UDP-Socket geoeffnet? */
	if (udp_deskriptor >= 0)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Already in game.", NULL }; /* Fehlermeldung */

		uebler_fehler(meldung, NULL);
	}

	/* falls server_name ein NULL-Zeiger ist, Namen des Default-Servers
	   verwenden */
	if (server_name == NULL)
		server_name = net_get_server_name(NULL);

	{
		char *trenn_position; /* Position des letzten Doppelpunktes in
		                         server_name, sonst NULL */

		/* letzten Doppelpunkt im Namen suchen */
		if ((trenn_position = strrchr(server_name, ':')) == NULL)
		{
			/* kein Doppelpunkt vorhanden, also: */

			/* Hostname ist gesamter String server_name */
			server_hostname = server_name;

			/* leerer Portname */
			server_portname = "";

			/* Hostname wurde nicht in extra Speicher kopiert */
			hostname_kopiert = 0;
		}
		else
		{
			int hostname_laenge; /* Laenge des Hostnamen */

			/* Doppelpunkt vorhanden */

			/* Laenge ist Anzahl der Zeichen vor der trenn_position */
			hostname_laenge = trenn_position - server_name;

			/* Speicher fuer Hostnamen plus Stringende-Kennung belegen */
			speicher_belegen((void **)&server_hostname,
				hostname_laenge + 1);

			/* Hostnamen kopieren und Stringende-Kennung setzen */
			strncpy(server_hostname, server_name, hostname_laenge);
			server_hostname[hostname_laenge] = 0;

			/* Portname beginnt hinter dem Doppelpunkt */
			server_portname = trenn_position + 1;

			/* Hostname wurde in extra Speicher kopiert */
			hostname_kopiert = 1;
		}
	}

	/* falls Hostname leerer String ist, durch "localhost" ersetzen */
	if (server_hostname[0] == 0)
	{
		/* falls Hostname bereits kopiert wurde, Speicher wieder
		   freigeben */
		if (hostname_kopiert)
		{
			speicher_freigeben((void **)&server_hostname);
			hostname_kopiert = 0;
		}

		server_hostname = "localhost";
	}

	/* komplette Adresse des Servers zusammenstellen: */
	memset(&server_tcp_adresse, 0, sizeof server_tcp_adresse);
	server_tcp_adresse.sin_family = AF_INET; /* Address Family setzen */

	/* falls Hostname IP-Nummer ist, speichern */
	if ((server_tcp_adresse.sin_addr.s_addr =
		inet_addr(server_hostname)) == 0xffffffff)
	{
		struct hostent *host_eintrag; /* Name/Nummer-Zuordnung des Hosts */

		/* Hostname ist keine IP-Nummer */

		/* Name/Nummer-Zuordnung mittels des Namen abfragen; falls
		   keine Zuordnung gefunden, Fehler */
		if ((host_eintrag = gethostbyname(server_hostname)) == NULL)
		{
			static char *meldung[] = { "iMaze - Network Error", "",
				"Unknown server host:", NULL, NULL }; /* Fehlermeldung */

			/* Hostnamen mit anzeigen */
			meldung[3] = server_hostname;

			milder_fehler(meldung, NULL);

			/* Speicher wieder freigeben */
			if (hostname_kopiert)
				speicher_freigeben((void **)&server_hostname);

			/* Fehler aufgetreten */
			return 1;
		}

		/* IP-Nummer des Servers kopieren */
		memcpy(&server_tcp_adresse.sin_addr, host_eintrag->h_addr_list[0],
			host_eintrag->h_length);
	}

	/* ist der Portname ein leerer String? */
	if (server_portname[0] == 0)
	{
		struct servent *service_eintrag; /* Portname/-nummer-Zuordnung */

		/* Portname/-nummer-Zuordnung des Ports "imaze" im Protokoll "tcp"
		   abfragen */
		if ((service_eintrag = getservbyname("imaze", "tcp")) != NULL)
			/* Nummer ist bekannt */
			server_tcp_adresse.sin_port = service_eintrag->s_port;
		else
			/* Nummer ist unbekannt, Default-Portnummer verwenden */
			server_tcp_adresse.sin_port = htons(DEFAULT_PORT);
	}
	else
	{
		/* Portname ist angegeben, als Nummer interpretieren und merken */
		if ((server_tcp_adresse.sin_port =
			htons(atoi(server_portname))) == 0)
		{
			struct servent *service_eintrag; /* Portname/-nummer-
			                                    Zuordnung */

			/* Portname war keine Nummer; Portname/-nummer-Zuordnung des
			   angegebenen Ports mittels des Portname abfragen; falls
			   keine Zuordnung gefunden, Fehler */
			if ((service_eintrag = getservbyname(server_portname,
				"tcp")) == NULL)
			{
				/* Fehlermeldung: */
				static char *meldung[] = { "iMaze - Network Error", "",
					"Unknown TCP service:", NULL, NULL };

				/* Portnamen mit anzeigen */
				meldung[3] = server_portname;

				milder_fehler(meldung, NULL);

				/* Speicher wieder freigeben */
				if (hostname_kopiert)
					speicher_freigeben((void **)&server_hostname);

				/* Fehler aufgetreten */
				return 1;
			}

			/* gefundene Nummer uebernehmen */
			server_tcp_adresse.sin_port = service_eintrag->s_port;
		}
	}

	/* falls Hostname kopiert wurde, Speicher wieder freigeben */
	if (hostname_kopiert)
		speicher_freigeben((void **)&server_hostname);

	/* TCP-Socket oeffnen */
	if ((tcp_deskriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		netzwerk_fehler("Can't create prolog socket:");

	/* TCP-Socket mit Server verbinden, falls fehlgeschlagen, Fehler */
	if (connect(tcp_deskriptor, (struct sockaddr *)&server_tcp_adresse,
		sizeof server_tcp_adresse))
	{
		/* Fehlermeldung: */
		static char *meldung[] = { "iMaze - Network Error", "",
			"Can't connect to server host:", NULL, "", NULL, NULL };

		/* server_name wie angegeben und Standard-Fehlertext mit anzeigen */
		meldung[3] = server_name;
		meldung[5] = fehler_text();

		milder_fehler(meldung, NULL);

		/* TCP-Deskriptor wieder freigeben */
		if (close(tcp_deskriptor))
			netzwerk_fehler("Can't close prolog socket:");

		tcp_deskriptor = -1;

		/* Fehler aufgetreten */
		return 1;
	}

	/* kein Fehler */
	return 0;
}


/*
** prolog_paket_senden
**  sendet ein Prolog-Paket
**
** Parameter:
**  paket_laenge: Laenge des Paketes
**  paket: Zeiger auf die Paketdaten
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
*/
int prolog_paket_senden(int paket_laenge, void *paket)
{
	u_char laenge_daten[2]; /* Puffer fuer die Laengenangabe des
	                           folgenden Paketes */

	/* noch keine TCP-Verbindung aufgebaut? */
	tcp_deskriptor_pruefen();

	/* Paketlaenge zu gross, um in zwei Bytes uebertragen zu werden? */
	if (paket_laenge > 65535)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Prolog packet too large:", NULL, NULL }; /* Fehlermeldung */
		char text[20]; /* Puffer fuer Text "xx bytes" */

		/* Paketlaenge mit anzeigen */
		sprintf(text, "%d bytes", paket_laenge);
		meldung[3] = text;

		uebler_fehler(meldung, NULL);
	}

	/* Paketlaenge im Puffer speichern, erst hoeherwertiger, dann
	   niederwertiger Teil */
	laenge_daten[0] = paket_laenge / 256;
	laenge_daten[1] = paket_laenge % 256;

	/* Puffer mit zwei Bytes Laengenangabe an Server schicken */
	if (puffer_schreiben(tcp_deskriptor, 2, laenge_daten) != 2)
	{
		milder_netzwerk_fehler("Can't send prolog packet length:");
		return 1;
	}

	/* falls Paketlaenge nicht 0, Paket selbst an Server schicken */
	if (paket_laenge)
		if (puffer_schreiben(tcp_deskriptor, paket_laenge,
			paket) != paket_laenge)
		{
			milder_netzwerk_fehler("Can't send prolog packet:");
			return 1;
		}

	/* kein Fehler */
	return 0;
}


/*
** prolog_paket_empfangen
**  empfaengt ein Prolog-Paket, wartet bis eins ankommt, falls noetig
**
** Parameter:
**  paket_laenge: Zeiger auf die Laenge des Paketes
**  paket: Zeiger auf einen Zeiger auf das Paket
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  *paket_laenge und *paket werden gesetzt; falls *paket_laenge nicht 0,
**  wird Speicher belegt, der von der aufrufenden Routine freigegeben
**  werden muss
*/
int prolog_paket_empfangen(int *paket_laenge, void **paket)
{
	u_char laenge_daten[2]; /* Puffer fuer die Laengenangabe des
	                           folgenden Paketes */

	/* noch keine TCP-Verbindung aufgebaut? */
	tcp_deskriptor_pruefen();

	/* zwei Bytes Laengenangabe vom Server empfangen */
	if (puffer_lesen(tcp_deskriptor, 2, laenge_daten) != 2)
	{
		*paket_laenge = 0;

		milder_netzwerk_fehler("Can't receive prolog packet length:");
		return 1;
	}

	/* Paketlaenge aus Puffer auslesen, erst hoeherwertiger, dann
	   niederwertiger Teil */
	*paket_laenge = (u_short)laenge_daten[0] * 256 |
		(u_short)laenge_daten[1];

	/* Paketlaenge nicht 0 ? */
	if (*paket_laenge)
	{
		/* Speicher fuer Paket selbst belegen */
		speicher_belegen(paket, *paket_laenge);

		/* Paket selbst vom Server empfangen */
		if (puffer_lesen(tcp_deskriptor, *paket_laenge,
			*paket) != *paket_laenge)
		{
			milder_netzwerk_fehler("Can't receive prolog packet:");
			return 1;
		}
	}

	return 0;
}


/*
** verbindung_auf_spiel
**  schaltet die Verbindung zum Server aus dem Prolog-Modus
**  in den Spiel-Modus; hier: baut TCP-Verbindung ab und oeffnet UDP-Socket
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  setzt tcp_deskriptor und letzte_paketnummer auf -1, setzt udp_deskriptor
*/
int verbindung_auf_spiel(void)
{
	struct sockaddr_in udp_adresse;        /* lokale Adresse des
	                                          UDP-Sockets */
	struct sockaddr_in server_udp_adresse; /* Adresse des UDP-Sockets
	                                          des Servers */
	u_char adresse_daten[6]; /* Puffer fuer die Adresse des
	                            Server-UDP-Sockets */

	/* bereits ein UDP-Socket geoeffnet? */
	if (udp_deskriptor >= 0)
	{
		static char *meldung[] = { "iMaze - Network Error", "",
			"Already in game.", NULL }; /* Fehlermeldung */

		uebler_fehler(meldung, NULL);
	}

	/* noch keine TCP-Verbindung aufgebaut? */
	tcp_deskriptor_pruefen();

	/* UDP-Socket oeffnen */
	if ((udp_deskriptor = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		netzwerk_fehler("Can't create game socket:");

	memset(&udp_adresse, 0, sizeof udp_adresse);
	udp_adresse.sin_family = AF_INET;         /* Address Family setzen */
	udp_adresse.sin_port = 0;                 /* Port ist beliebig */
	udp_adresse.sin_addr.s_addr = INADDR_ANY; /* eigene Hostadresse ist
	                                             beliebig */

	/* UDP-Socket lokal an beliebigen Port binden */
	if (bind(udp_deskriptor, (struct sockaddr *)&udp_adresse,
		sizeof udp_adresse))
		netzwerk_fehler("Can't bind game socket to any address:");

	/* Adresse des Server-UDP-Sockets in 6 Bytes vom Server empfangen */
	if (puffer_lesen(tcp_deskriptor, 6, adresse_daten) != 6)
	{
		milder_netzwerk_fehler("Can't receive game address:");
		return 1;
	}

	/* Adresse des Servers aus den 6 Bytes in einen struct sockaddr_in
	   umwandeln; die ersten 4 Bytes sind die IP-Nummer des Servers, dann
	   der hoeherwertige und der niederwertige Teil der Portnummer in je
	   einem Byte */
	memset(&server_udp_adresse, 0, sizeof server_udp_adresse);
	server_udp_adresse.sin_family = AF_INET;
	server_udp_adresse.sin_port = htons((u_short)adresse_daten[4] << 8 |
		(u_short)adresse_daten[5]);
	server_udp_adresse.sin_addr.s_addr =
		htonl((u_long)adresse_daten[0] << 24 |
			(u_long)adresse_daten[1] << 16 | (u_long)adresse_daten[2] << 8 |
			(u_long)adresse_daten[3]);

	/* jetzt durch den UDP-Socket nur noch Pakete an die Adresse des
	   Servers schicken und nur von derselben Adresse empfangen */
	if (connect(udp_deskriptor, (struct sockaddr *)&server_udp_adresse,
		sizeof server_udp_adresse))
	{
		milder_netzwerk_fehler("Can't connect game socket to game address:");
		return 1;
	}

	{
		ADDRLEN_T adresslaenge; /* temporaerer Speicher fuer die
		                           Groesse eines struct sockaddr_in */
		u_short udp_port; /* Port, an den der UDP-Socket lokal
		                     gebunden ist */

		/* tatsaechlich zugeordnete lokale IP-Nummer/Portnummer des
		   UDP-Sockets abfragen */
		adresslaenge = sizeof udp_adresse;
		if (getsockname(udp_deskriptor, (struct sockaddr *)&udp_adresse,
			&adresslaenge))
			netzwerk_fehler("Can't figure out game socket address:");

		/* UDP-Port merken */
		udp_port = udp_adresse.sin_port;

		/* lokale IP-Nummer/Portnummer der TCP-Verbindung abfragen */
		adresslaenge = sizeof udp_adresse;
		if (getsockname(tcp_deskriptor, (struct sockaddr *)&udp_adresse,
			&adresslaenge))
			netzwerk_fehler("Can't figure out prolog socket address:");

		/* IP-Nummer von der TCP-Verbindung uebernehmen, Portnummer von
		   dem UDP-Socket; die IP-Nummer des UDP-Sockets kann noch
		   undefiniert sein */
		udp_adresse.sin_port = udp_port;
	}

	/* lokale Adresse des UDP-Sockets aus dem struct sockaddr_int in
	   6 Bytes umwandeln; die ersten 4 Bytes sind die IP-Nummer, dann
	   der hoeherwertige und der niederwertige Teil der Portnummer in je
	   einem Byte */
	adresse_daten[0] = ntohl(udp_adresse.sin_addr.s_addr) >> 24;
	adresse_daten[1] = ntohl(udp_adresse.sin_addr.s_addr) >> 16;
	adresse_daten[2] = ntohl(udp_adresse.sin_addr.s_addr) >> 8;
	adresse_daten[3] = ntohl(udp_adresse.sin_addr.s_addr);
	adresse_daten[4] = ntohs(udp_adresse.sin_port) >> 8;
	adresse_daten[5] = ntohs(udp_adresse.sin_port);

	/* Adresse des UDP-Sockets in 6 Bytes zum Server schicken */
	if (puffer_schreiben(tcp_deskriptor, 6, adresse_daten) != 6)
	{
		milder_netzwerk_fehler("Can't send game socket address:");
		return 1;
	}

	/* TCP-Verbindung abbauen */
	if (close(tcp_deskriptor))
		netzwerk_fehler("Can't close prolog socket:");

	/* TCP-Deskriptor ist nicht mehr gueltig */
	tcp_deskriptor = -1;

	/* UDP-Socket in einen Modus versetzen, in dem das Empfangen eines
	   Pakets den Prozess nicht blockiert, wenn noch kein Paket bereit
	   ist, sondern einen Fehlercode erzeugt; siehe Routine paket_handler */
	if (deskriptor_nicht_blockieren(udp_deskriptor))
		netzwerk_fehler("Can't switch game socket to non-blocking mode:");

	/* noch kein Spiel-Paket empfangen, daher keine Paketnummer bekannt */
	letzte_paketnummer = -1;

	return 0;
}


/*
** spiel_deskriptor
**  liefert einen Deskriptor fuer die Verbindung zum Server im Spiel-Modus;
**  hier: den udp_deskriptor
**
** Rueckgabewert:
**  ein Zeiger auf den Deskriptor, NULL, falls es noch keine gibt
**
** Seiteneffekte:
**  der Zeiger zeigt auf eine static-Variable, die jedesmal ueberschrieben
**  wird
*/
void *spiel_deskriptor(void)
{
	static int deskriptor;

	/* noch kein UDP-Socket geoeffnet? */
	if (udp_deskriptor < 0)
		return NULL;

	deskriptor = udp_deskriptor;

	return &deskriptor;
}


/*
** spiel_paket_senden
**  sendet ein Prolog-Paket
**
** Parameter:
**  paket_laenge: Laenge des Paketes
**  paket: Zeiger auf die Paketdaten
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
*/
int spiel_paket_senden(int paket_laenge, void *paket)
{
	u_char puffer[MAX_SPIEL_PAKET]; /* Puffer fuer Gesamtpaket inklusive
	                                   2 Bytes Laengenangabe */

	/* noch kein UDP-Socket geoeffnet? */
	udp_deskriptor_pruefen();

	/* falls das Paket zusammen mit der Laengenangabe nicht in den Puffer
	   passt, das Paket stillschweigend ignorieren */
	if (paket_laenge + 2 > sizeof puffer)
		/* keinen Fehler melden */
		return 0;

	/* die Laengenangabe mit hoeherwertigem und niederwertigem Teil in
	   je ein Byte und dahinter das Paket in den Puffer kopieren */
	puffer[0] = letzte_paketnummer / 256;
	puffer[1] = letzte_paketnummer % 256;
	memcpy(puffer + 2, paket, paket_laenge);

	/* Gesamtpaket inklusive 2 Bytes Laengenangabe an Server senden */
	paket_laenge += 2;
	if (send(udp_deskriptor, (void *)puffer, paket_laenge,
		0) != paket_laenge)
	{
		milder_netzwerk_fehler("Can't send game packet:");
		return 1;
	}

	/* kein Fehler aufgetreten */
	return 0;
}


/*
** spiel_paket_erwarten
**  wartet darauf, dass ein Spiel-Paket vom Server ankommt
**
** Parameter:
**  ms: Zeit in Millisekunden, die maximal gewartet werden darf;
**      oder -1 fuer unbegrenztes Warten
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
*/
int spiel_paket_erwarten(int ms)
{
	struct timeval timeout; /* Timeout in Sekunden/Mikrosekunden */
	fd_set deskriptoren;    /* Deskriptoren, auf die gewartet werden soll */

	/* noch kein UDP-Socket geoeffnet? */
	udp_deskriptor_pruefen();

	/* falls Timeout nicht -1, in Sekunden/Mikrosekunden wandeln */
	if (ms >= 0)
	{
		timeout.tv_sec = ms / 1000;
		timeout.tv_usec = (ms % 1000) * 1000;
	}

	/* nur auf den udp_deskriptor warten */
	FD_ZERO(&deskriptoren);
	FD_SET(udp_deskriptor, &deskriptoren);

	/* mit Timeout (falls ms nicht -1) darauf warten, dass ein Paket
	   durch den udp_deskriptor gelesen werden kann */
	switch (select(max_deskriptor_anzahl(), &deskriptoren, NULL, NULL,
		ms < 0 ? NULL : &timeout))
	{
		case -1: /* Fehler */
			/* Fehler EINTR (Interrupted System Call) ignorieren */
			if (errno == EINTR)
				break;

			/* bei anderem Fehler Fehlerroutine aufrufen */
			netzwerk_fehler("Can't wait for game packet:");

		case 0: /* Timeout */
			/* bei Timeout nichts tun */
			break;

		default: /* Deskriptor bereit */
			/* angekommene Pakete an den uebergeordneten Netzwerkteil
			   weiterreichen */
			if (paket_handler())
				return 1;
	}

	/* kein Fehler */
	return 0;
}


/*
** verbindung_beenden
**  beendet die Verbindung zum Server, egal ob im Prolog- oder Spiel-Modus
**
** Seiteneffekte:
**  setzt tcp_deskriptor und udp_deskriptor auf -1
*/
void verbindung_beenden(void)
{
	/* ist eine TCP-Verbindung aufgebaut? */
	if (tcp_deskriptor >= 0)
	{
		/* TCP-Verbindung abbauen */
		if (close(tcp_deskriptor))
			netzwerk_fehler("Can't close prolog socket:");

		/* TCP-Deskriptor ist nicht mehr gueltig */
		tcp_deskriptor = -1;
	}

	/* ist ein UDP-Socket geoeffnet? */
	if (udp_deskriptor >= 0)
	{
		/* UDP-Socket schliessen */
		if (close(udp_deskriptor))
			netzwerk_fehler("Can't close game socket:");

		/* UDP-Deskriptor ist nicht mehr gueltig */
		udp_deskriptor = -1;
	}
}


/*
** netzwerk_ende
**  beendet den Netzwerkteil; hier: die Verbindung wird beendet
*/
void netzwerk_ende(void)
{
	/* es gibt keine lokalen Datenstrukturen aufzuraeumen; einfach die
	   Verbindung zum Server beenden, falls vorhanden */
	verbindung_beenden();
}
