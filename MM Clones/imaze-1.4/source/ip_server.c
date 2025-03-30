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
** Datei: ip_server.c
**
** Kommentar:
**  Netzwerkkommunikation ueber TCP/IP und
**  Interprozesskommunikation ueber Sockets
*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <netdb.h>

#include "argv.h"
#include "speicher.h"
#include "server_netz.h"
#include "system.h"
#include "fehler.h"

static char sccsid[] = "@(#)ip_server.c	3.19 12/05/01";


/* Discard-Port (Pfusch!) */
#ifndef IPPORT_DISCARD
#define IPPORT_DISCARD 9
#endif


/* Default-TCP-Port des Servers */
#ifndef DEFAULT_PORT
#define DEFAULT_PORT /* phone +49-*/5323/*-72-3896 */
#endif

/* Default-UDP-Port des Servers */
#ifndef SERVER_UDP_PORT
#define SERVER_UDP_PORT DEFAULT_PORT
#endif

#ifndef ADDRLEN_T
#define ADDRLEN_T int
#endif


static char *log_datei_name = NULL;
static char *portname = NULL;
struct arg_option net_opts[] =
{
	{ Arg_String, "l", &log_datei_name, "set log file, - = stdout",
		"{file|-}" },
	{ Arg_String, "p", &portname, "set server TCP port", "port" },
	{ Arg_End }
};


/* Makro, das aus einem Verbindungsdeskriptor den zugehoerigen
   Prolog-Socket ausliest */
#define SOCKET(verbindungs_deskriptor) \
	(((struct verbindung *)(verbindungs_deskriptor))->socket)


/* Struktur, in der alle relevanten Daten ueber eine Verbindung
   zu einem Client gespeichert werden */
struct verbindung
{
	struct sockaddr_in adresse; /* IP-Adresse des Clients */
	int socket;    /* Deskriptor des Sockets, ueber den die
	                  Kommunikation laeuft (nur fuer den Prolog) */
	int auf_spiel; /* Flag, ob die Verbindung vom Prolog auf das Spiel
	                  umgeschaltet wurde */
	int original;  /* Flag, ob dieser Deskriptor das Original ist und
	                  noch nicht als Nachricht an einen anderen Prozess
	                  gesandt wurde */
};

/* Struktur, die einen Client beschreibt */
struct nachricht_verbindung
{
	struct sockaddr_in client_adresse; /* IP-Adresse des Clients */
	int auf_spiel; /* Client ist schon in der Spiel-Phase */
};

/* Struktur, die beschreibt, welche Teile der Nechricht vorhanden sind
   bzw. welche Groesse sie haben */
struct nachricht_kopf
{
	int laenge;      /* Groesse des angehaengten Datenteils */
	char verbindung; /* Verbindungsbeschreibung vorhanden? */
};

/* Struktur, in der eine Nachricht fuer die
   Interprozesskommunikation gespeichert werden kann */
struct nachricht
{
	struct nachricht_kopf kopf; /* Kopf der Nachricht */
	struct nachricht_verbindung verbindung; /* Beschreibung des Clients;
	                                           nur vorhanden, wenn
	                                           kopf.verbindung 1 ist */
};


/* Systemvariable: letzter aufgetretener Fehlercode oder 0 */
extern int errno;

FILE *log_deskriptor; /* Deskriptor fuer das Log-File */

/* im Server-Prozess: */
static int server_prolog_socket; /* Socket, auf dem die Prolog-Verbindungen
                                    der Clients entgegengenommen werden */

/* im Session-Prozess: */
static int socket_zum_server;    /* Socket fuer die Kommunikation mit
                                    dem Server-Prozess */
static int session_spiel_socket; /* Socket, ueber den mit den Clients die
                                    Spiel-Pakete ausgetauscht werden */

/* im Initialisierungs-Prozess: */
static void *init_verbindung = NULL; /* Verbindung, auf der der Prozess
                                        den Client initialisiert */


/*
** prozess_ende
**  wird aufgerufen, wenn ein Prozess stirbt; vermeidet Zombies
**
** Parameter:
**  signum: Dummy-Parameter
*/
void prozess_ende(int signum)
{
	/* Statuswerte der toten Prozesse abfragen */
	while (waitpid(-1, NULL, WNOHANG) > 0);
}


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
** verbindung_init
**  belegt Speicher und initialisiert eine Struktur zur Beschreibung
**  der Verbindung zu einem Client
**
** Parameter:
**  adresse: Zeiger auf die IP-Adresse des Clients
**
** Rueckgabewert:
**  Zeiger auf die Beschreibungsstruktur
*/
static struct verbindung *verbindung_init(struct sockaddr_in *adresse)
{
	struct verbindung *verbindung; /* Zeiger auf die Struktur */

	/* Speicher belegen */
	speicher_belegen((void **)&verbindung, sizeof(struct verbindung));

	/* Adresse kopieren */
	memcpy(&verbindung->adresse, adresse, sizeof(struct sockaddr_in));

	/* Zeiger zurueckgeben */
	return verbindung;
}


/*
** verbindungsname
**  wandelt die Adresse einer Verbindung in einen String um
**
** Parameter:
**  verbindung: die Verbindung, deren Adresse umgewandelt werden soll
**
** Rueckgabewert:
**  String, der die Adresse beschreibt
**
** Seiteneffekte:
**  der Strings wird jedesmal ueberschrieben
*/
static char *verbindungsname(struct verbindung *verbindung)
{
	static char name[40];

	sprintf(name, "%s:%d/%s", inet_ntoa(verbindung->adresse.sin_addr),
		ntohs(verbindung->adresse.sin_port),
		verbindung->auf_spiel ? "udp" : "tcp");

	return name;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** prolog_paket_senden
**  sendet ein Prolog-Paket zu einem Client
**
** Parameter:
**  verbindung: Deskriptor fuer die Verbindung zum Client
**  paket_laenge: Laenge des Pakets
**  paket: das Paket
**
** Rueckgabewert:
**  1, falls ein Fehler aufgetreten ist, sonst 0
*/
int prolog_paket_senden(void *verbindung, int paket_laenge, void *paket)
{
	unsigned char laenge[2]; /* Paket, in dem die Laenge des Datenpaketes
	                            gesendet wird */

	/* Laengen ueber 65535 sind nicht moeglich */
	if (paket_laenge > 65535)
		return 1;

	/* Laenge der folgenden Daten senden */
	laenge[0] = paket_laenge / 256;
	laenge[1] = paket_laenge % 256;
	if (puffer_schreiben(SOCKET(verbindung), 2, laenge) != 2)
		return 1;

	/* falls Laenge der Daten nicht 0 ist, die Daten senden */
	if (paket_laenge)
		if (puffer_schreiben(SOCKET(verbindung),
			paket_laenge, paket) != paket_laenge)
			return 1;

	/* kein Fehler aufgetreten */
	return 0;
}


/*
** prolog_paket_empfangen
**  empfaengt ein Prolog-Paket von einem Client;
**  belegt Speicher dafuer, der spaeter freigegeben werden muss
**
** Parameter:
**  verbindung: Deskriptor fuer die Verbindung zum Client
**  paket_laenge: Zeiger auf die Laenge des Pakets
**  paket: Zeiger auf das Paket
**
** Rueckgabewert:
**  1, falls ein Fehler aufgetreten ist, sonst 0
*/
int prolog_paket_empfangen(void *verbindung, int *paket_laenge, void **paket)
{
	unsigned char laenge[2]; /* Puffer fuer die Laenge der folgenden Daten */

	/* erst die Laenge der darauffolgenden Daten empfangen */
	if (puffer_lesen(SOCKET(verbindung), 2, laenge) != 2)
		return 1;
	*paket_laenge = (unsigned short)laenge[0] * 256 |
		(unsigned short)laenge[1];

	/* Speicher fuer das Paket belegen */
	speicher_belegen(paket, *paket_laenge);

	/* die Daten empfangen */
	if (puffer_lesen(SOCKET(verbindung),
		*paket_laenge, *paket) != *paket_laenge)
		return 1;

	/* kein Fehler aufgetreten */
	return 0;
}


/*
** spiel_paket_senden
**  sendet in der Spiel-Phase ein Paket an einen Client
**
** Parameter:
**  verbindung: Deskriptor fuer die Verbindung zum Client
**  paket_laenge: Laenge des Pakets
**  paket: das Paket
*/
void spiel_paket_senden(void *verbindung, int paket_laenge, void *paket)
{
	/* Paket senden, Fehler (wie temporaeren
	   Puffermangel, ENOBUFS) ignorieren */
	sendto(session_spiel_socket, paket, paket_laenge, 0,
		(struct sockaddr *)&((struct verbindung *)verbindung)->adresse,
		sizeof(struct sockaddr_in));
}


/*
** spiel_paket_empfangen
**  empfaengt in der Spiel-Phase ein Paket von einem der Clients;
**  belegt Speicher dafuer, der spaeter freigegeben werden muss
**
** Parameter:
**  verbindungen_anz: Anzahl der Clients
**  verbindungen: Feld mit Deskriptoren fuer die Verbindungen
**  paket_laenge: Zeiger auf die Laenge des Pakets
**  paket: Zeiger auf das Paket
**
** Rueckgabewert:
**  -1 bei Fehler, sonst einen Index in das Feld verbindungen
*/
int spiel_paket_empfangen(int verbindungen_anz, void **verbindungen,
	int *paket_laenge, void **paket)
{
	int i;                            /* Index des Clients */
	ADDRLEN_T paket_adresse_laenge;   /* Groesse der IP-Adresse */
	struct sockaddr_in paket_adresse; /* IP-Adresse */
	char paket_tmp[1024];             /* Puffer fuer das Paket */

	/* Schleife, bis ein Paket zugeordnet werden konnte oder
	   keins mehr bereitsteht */
	for (;;)
	{
		/* Paket empfangen */
		paket_adresse_laenge = sizeof paket_adresse;
		*paket_laenge = recvfrom(session_spiel_socket, paket_tmp,
			sizeof paket_tmp, 0, (struct sockaddr *)&paket_adresse,
			&paket_adresse_laenge);

		/* Fehler? */
		if (*paket_laenge <= 0)
		{
			/* kein Paket mehr bereit? */
			if (errno == EWOULDBLOCK || errno == EAGAIN ||
				*paket_laenge == 0)
				return -1;

			/* bei anderem Fehler Programm mit Fehlermeldung beenden */
			netzwerk_fehler("Can't receive game packet:");
		}

		/* Client suchen, dem die IP-Adresse gehoert */
		for (i = 0; i < verbindungen_anz &&
			(((struct verbindung *)verbindungen[i])->
			adresse.sin_port != paket_adresse.sin_port ||
			((struct verbindung *)verbindungen[i])->
			adresse.sin_addr.s_addr != paket_adresse.sin_addr.s_addr);
			i++);

		/* gefunden? dann Schleife verlassen */
		if (i < verbindungen_anz)
			break;
	}

	/* Speicher fuer Paket belegen und kopieren */
	speicher_belegen(paket, *paket_laenge);
	memcpy(*paket, paket_tmp, *paket_laenge);

	/* Index zurueckgeben */
	return i;
}


/*
** verbindung_freigeben
**  gibt einen Verbindungsdeskriptor wieder frei;
**  beendet evtl. den Initialisierungs-Prozess
**
** Parameter:
**  verbindung: Deskriptor fuer die Verbindung zum Client
**
** Seiteneffekte:
**  der Speicher fuer *verbindung wird freigegeben;
**  evtl. wird dieser Prozess beendet
*/
void verbindung_freigeben(void *verbindung)
{
	/* mitloggen */
	if (((struct verbindung *)verbindung)->auf_spiel &&
		((struct verbindung *)verbindung)->original &&
		log_deskriptor != NULL)
		fprintf(log_deskriptor, "%s: player has left on %s\n",
			uhrzeit(), verbindungsname(verbindung));

	/* ist dies der zugehoerige Initialisierungs-Prozess? */
	if (verbindung == init_verbindung)
	{
		/* den Speicher freigeben */
		speicher_freigeben(&verbindung);

		/* und den Prozess beenden */
		exit(0);
	}

	/* sonst nur den Speicher freigeben */
	speicher_freigeben(&verbindung);
}


/*
** verbindung_annehmen
**  nimmt eine Prolog-Verbindung zu einem Client entgegen
**
** Rueckgabewert:
**  Deskriptor fuer die Verbindung zum Client; NULL, falls
**  keine Verbindung bereit war oder ein Initialisierungs-Prozess
**  abgespalten wurde und dies der Server-Prozess ist
*/
void *verbindung_annehmen(void)
{
	int deskriptor;                    /* IP-Deskriptor der Verbindung */
	ADDRLEN_T client_adresse_laenge;   /* Groesse der IP-Adresse */
	struct sockaddr_in client_adresse; /* IP-Adresse */
	struct verbindung *verbindung;     /* Deskriptor der Verbindung */

	/* Verbindung entgegennehmen */
	client_adresse_laenge = sizeof client_adresse;
	if ((deskriptor = accept(server_prolog_socket,
		(struct sockaddr *)&client_adresse,
		&client_adresse_laenge)) < 0)
	{
		/* keine Verbindung bereit? */
		if (errno == EINTR || errno == EWOULDBLOCK ||
			errno == EAGAIN)
			return NULL;

		/* bei anderem Fehler Programm mit Fehlermeldung beenden */
		netzwerk_fehler("Can't accept connection from client:");
	}

	/* Initialisierungs-Prozess abspalten */
	switch (fork())
	{
		case -1: /* Fehler */
			netzwerk_fehler("Can't fork client initialization process:");

		case 0: /* Initialisierungs-Prozess */
			/* Deskriptor initialisieren */
			verbindung = verbindung_init(&client_adresse);
			verbindung->socket = deskriptor;
			verbindung->auf_spiel = 0;

			/* merken, dass dies der zu verbindung gehoerige
			   Initialisierungs-Prozess ist */
			init_verbindung = verbindung;

			/* mitloggen */
			if (log_deskriptor != NULL)
				fprintf(log_deskriptor, "%s: connection from %s\n",
					uhrzeit(), verbindungsname(verbindung));

			/* Verbindungs-Deskriptor zurueckgeben */
			return verbindung;

		default: /* Server-Prozess */
			/* Verbindung wieder schliessen */
			close(deskriptor);

			/* keine Verbindung zurueckgeben */
			return NULL;
	}
}


/*
** verbindung_abbrechen
**  bricht waehrend der Prolog-Phase die Verbindung ab und
**  beendet den zugehoerigen Initialisierungs-Prozess
**
** Parameter:
**  verbindung: Deskriptor fuer die Verbindung zum Client
**  fehler: als Fehler loggen?
**
** Seiteneffekte:
**  gibt den Speicher fuer *verbindung frei und
**  beendet den Initialisierungs-Prozess
*/
void verbindung_abbrechen(void *verbindung, int fehler)
{
	/* mitloggen */
	if (log_deskriptor != NULL)
		fprintf(log_deskriptor, "%s: %s on %s\n",
			uhrzeit(), fehler ? "init failed" : "query done",
			verbindungsname(verbindung));

	/* Verbindung schliessen */
	close(SOCKET(verbindung));

	/* Speicher freigeben */
	speicher_freigeben(&verbindung);

	/* Initialisierungs-Prozess beenden */
	exit(0);
}


/*
** verbindung_auf_spiel
**  schaltet eine Verbindung aus der Prolog-Phase in die
**  Spiel-Phase
**
** Parameter:
**  session: Session, zu der der Client verbunden werden soll
**  verbindung: Deskriptor fuer die Verbindung zum Client
**
** Rueckgabewert:
**  1, falls ein Fehler aufgetreten ist, sonst 0
*/
int verbindung_auf_spiel(void *session, void *verbindung)
{
	unsigned char port[6];                  /* Puffer zum Uebertragen der
	                                           Adresse ueber das Netz */
	ADDRLEN_T spiel_adresse_laenge;         /* Grosse der IP-Adresse */
	struct sockaddr_in spiel_adresse;       /* IP-Adresse */
	struct sockaddr_in *verbindung_adresse; /* temporaerer Zeiger */
	struct verbindung alte_verbindung;      /* alte Verbindung (fuer
	                                           das Mitloggen) */

	{
		int tmp_deskriptor; /* Deskriptor fuer temporaere Verbindung */

		/* Adresse des Clients abfragen */
		spiel_adresse_laenge = sizeof spiel_adresse;
		if (getpeername(SOCKET(verbindung),
			(struct sockaddr *)&spiel_adresse,
			&spiel_adresse_laenge))
			return 1;

		/* Port auf Discard setzen */
		spiel_adresse.sin_port = htons(IPPORT_DISCARD);

		/* UDP/TCP-Deskriptor erzeugen */
#ifdef USE_TCP_DISCARD
		if ((tmp_deskriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
#else
		if ((tmp_deskriptor = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
#endif
			return 1;

		/* Verbindung zum Discard-Port des Clients aufbauen */
		if (connect(tmp_deskriptor, (struct sockaddr *)&spiel_adresse,
			sizeof spiel_adresse))
		{
			close(tmp_deskriptor);
			return 1;
		}

		/* eigene Adresse abfragen */
		spiel_adresse_laenge = sizeof spiel_adresse;
		if (getsockname(tmp_deskriptor,
			(struct sockaddr *)&spiel_adresse,
			&spiel_adresse_laenge))

			/* kein Erfolg? dann Adresse auf unbekannt setzen */
			spiel_adresse.sin_addr.s_addr = 0;

		/* Deskriptor wieder schliessen */
		close(tmp_deskriptor);
	}

	/* Adresse unbekannt? */
	if (spiel_adresse.sin_addr.s_addr == 0)
	{
		/* eigene Adresse der Prolog-Verbindung abfragen */
		spiel_adresse_laenge = sizeof spiel_adresse;
		if (getsockname(SOCKET(verbindung),
			(struct sockaddr *)&spiel_adresse,
			&spiel_adresse_laenge))

			/* Adresse nicht festzustellen */
			return 1;
	}

	/* die Adresse ist die eben abgefragte */
	port[0] = ntohl(spiel_adresse.sin_addr.s_addr) >> 24;
	port[1] = ntohl(spiel_adresse.sin_addr.s_addr) >> 16;
	port[2] = ntohl(spiel_adresse.sin_addr.s_addr) >> 8;
	port[3] = ntohl(spiel_adresse.sin_addr.s_addr);

	/* der Port haengt von der Session ab */
	port[4] = ((int *)session)[1] >> 8;
	port[5] = ((int *)session)[1];

	/* IP-Adresse des Session-Prozesses an den Client senden */
	if (puffer_schreiben(SOCKET(verbindung), 6, port) != 6)
		return 1;

	/* IP-Adresse des Clients fuer die Spiel-Phase empfangen */
	if (puffer_lesen(SOCKET(verbindung), 6, port) != 6)
		return 1;

	/* Prolog-Verbindung schliessen */
	if (close(SOCKET(verbindung)))
		return 1;

	/* alte Verbindung merken */
	memcpy(&alte_verbindung, verbindung, sizeof alte_verbindung);

	/* Zeiger auf die IP-Adresse im Verbindungs-Deskriptor */
	verbindung_adresse = &((struct verbindung *)verbindung)->adresse;

	/* IP-Adresse des Clients speichern */
	memset(verbindung_adresse, 0, sizeof *verbindung_adresse);
	verbindung_adresse->sin_family = AF_INET;
	verbindung_adresse->sin_port =
		htons((unsigned short)port[4] << 8 | (unsigned short)port[5]);
	verbindung_adresse->sin_addr.s_addr =
		htonl((unsigned long)port[0] << 24 | (unsigned long)port[1] << 16 |
		(unsigned long)port[2] << 8 | (unsigned long)port[3]);

	/* Verbindung ist jetzt auf Spiel geschaltet */
	((struct verbindung *)verbindung)->auf_spiel = 1;

	/* dieser Deskriptor ist das Original */
	((struct verbindung *)verbindung)->original = 1;

	/* mitloggen */
	if (log_deskriptor != NULL)
	{
		fprintf(log_deskriptor, "%s: %s", uhrzeit(),
			verbindungsname(&alte_verbindung));
		fprintf(log_deskriptor, " switched to %s\n",
			verbindungsname(verbindung));
		fprintf(log_deskriptor, "%s: %s plays on session %d/udp\n",
			uhrzeit(), verbindungsname(verbindung), ((int *)session)[1]);
	}

	/* kein Fehler aufgetreten */
	return 0;
}


/*
** netzwerk_init
**  initialisiert den Netzwerkteil des Server-Prozesses
*/
void netzwerk_init(void)
{
	struct sockaddr_in server_adresse; /* IP-Adresse des TCP-Sockets */

	log_deskriptor = NULL; /* noch kein Deskriptor vorhanden */

	/* Name fuer Log-File angegeben? */
	if (log_datei_name != NULL)
	{
		int tmp_deskriptor; /* temporaerer Deskriptor fuer das Log-File */

		/* bei "-" nach stdout loggen */
		if (!strcmp(log_datei_name, "-"))
			log_deskriptor = stdout;

		/* sonst Log-File oeffnen/erzeugen */
		else if ((tmp_deskriptor = open(log_datei_name,
			O_WRONLY|O_APPEND|O_CREAT, 0666)) < 0 ||
			(log_deskriptor = fdopen(tmp_deskriptor, "a")) == NULL)
		{
			char *text; /* Fehlertext */

			speicher_belegen((void **)&text, strlen(log_datei_name) + 20);
			sprintf(text, "Can't open \"%s\":", log_datei_name);
			netzwerk_fehler(text);
		}
	}

	/* IP-Adresse des TCP-Sockets initialisieren */
	memset(&server_adresse, 0, sizeof server_adresse);
	server_adresse.sin_family = AF_INET;
	server_adresse.sin_addr.s_addr = INADDR_ANY;

	/* kein Port angegeben? */
	if (portname == NULL)
	{
		struct servent *service_eintrag; /* Portname/-nummer-Zuordnung */

		/* Portname/-nummer-Zuordnung des Ports "imaze" im Protokoll "tcp"
		   abfragen */
		if ((service_eintrag = getservbyname("imaze", "tcp")) != NULL)
			/* Nummer ist bekannt */
			server_adresse.sin_port = service_eintrag->s_port;
		else
			/* Nummer ist unbekannt, Default-Portnummer verwenden */
			server_adresse.sin_port = htons(DEFAULT_PORT);
	}
	else
	{
		/* Portname ist angegeben, als Nummer interpretieren und merken */
		if ((server_adresse.sin_port = htons(atoi(portname))) == 0)
		{
			struct servent *service_eintrag; /* Portname/-nummer-
			                                    Zuordnung */

			/* Portname war keine Nummer; Portname/-nummer-Zuordnung des
			   angegebenen Ports mittels des Portname abfragen; falls
			   keine Zuordnung gefunden, Fehler */
			if ((service_eintrag = getservbyname(portname, "tcp")) == NULL)
			{
				/* Fehlermeldung: */
				static char *meldung[] = { "iMaze - Network Error", "",
					"Unknown TCP service:", NULL, NULL };

				/* Portnamen mit anzeigen */
				meldung[3] = portname;

				uebler_fehler(meldung, NULL);
			}

			/* gefundene Nummer uebernehmen */
			server_adresse.sin_port = service_eintrag->s_port;
		}
	}

	/* TCP-Socket erzeugen */
	if ((server_prolog_socket = socket(PF_INET, SOCK_STREAM,
		IPPROTO_TCP)) < 0)
		netzwerk_fehler("Can't create prolog socket:");

	/* Wiederverwenden des Ports immer erzwingen (in Servern) */
	{
		int eins = 1; /* Konstante 1 */

		/* entsprechende Option setzen */
		if (setsockopt(server_prolog_socket, SOL_SOCKET,
			SO_REUSEADDR, (void *)&eins, sizeof eins))
			netzwerk_fehler("Can't enable address reuse on prolog socket:");
	}

	/* TCP-Socket an gewuenschte Adresse binden */
	if (bind(server_prolog_socket, (struct sockaddr *)&server_adresse,
		sizeof server_adresse))
	{
		char fehler_text[80]; /* Fehlermeldung */

		/* Fehlermeldung erzeugen */
		sprintf(fehler_text, "Can't bind prolog socket to port %d:",
			ntohs(server_adresse.sin_port));

		/* Fehlermeldung anzeigen */
		netzwerk_fehler(fehler_text);
	}

	/* auf dem TCP-Socket ankommende Verbindungen zulassen */
	if (listen(server_prolog_socket, 5))
		netzwerk_fehler("Can't listen on prolog socket:");

	/* Handler fuer tote Prozesse installieren */
	handle_signal(SIGCHLD, prozess_ende);

	/* mitloggen? */
	if (log_deskriptor != NULL)
	{
		/* Deskriptor auf Zeilenpufferung schalten */
		setvbuf(log_deskriptor, NULL, _IOLBF, BUFSIZ);

		/* Start des Servers mitloggen */
		fprintf(log_deskriptor, "%s: server started\n", uhrzeit());
	}
}


/*
** session_starten
**  spaltet einen Session-Prozess ab
**
** Rueckgabewert:
**  ein Deskriptor fuer die Kommunikation mit dem Session-Prozess
**  bzw. NULL, wenn dies der Session-Prozess ist
*/
void *session_starten(void)
{
	int sockpair[2];     /* Sockets fuer die Interprozess-Kommunikation */
	int session_prozess; /* Prozess-ID des Session-Prozesses */
	struct sockaddr_in session_adresse; /* IP-Adresse (unspezifiziert) */

	/* UDP-Socket fuer die Verbindung zu den Clients in
	   der Spiel-Phase erzeugen */
	if ((session_spiel_socket = socket(PF_INET, SOCK_DGRAM,
		IPPROTO_UDP)) < 0)
		netzwerk_fehler("Can't create session socket:");

	/* UDP-Socket an vorgegebenen Port binden */
	memset(&session_adresse, 0, sizeof session_adresse);
	session_adresse.sin_family = AF_INET;
	session_adresse.sin_port = htons(SERVER_UDP_PORT);
	session_adresse.sin_addr.s_addr = INADDR_ANY;
	if (bind(session_spiel_socket, (struct sockaddr *)&session_adresse,
		sizeof session_adresse))
		if (errno == EADDRINUSE || errno == EACCES)
		{
			/* UDP-Socket an einen beliebigen Port binden */
			memset(&session_adresse, 0, sizeof session_adresse);
			session_adresse.sin_family = AF_INET;
			session_adresse.sin_port = 0;
			session_adresse.sin_addr.s_addr = INADDR_ANY;
			if (bind(session_spiel_socket,
				(struct sockaddr *)&session_adresse,
				sizeof session_adresse))
				netzwerk_fehler("Can't bind session socket to any address:");
		}
		else
		{
			char fehlermeldung[80];

			sprintf(fehlermeldung,
				"Can't bind session socket to port %d:",
				SERVER_UDP_PORT);
			netzwerk_fehler(fehlermeldung);
		}

	/* UDP-Socket in einen Modus versetzen, in dem das Empfangen eines
	   Pakets den Prozess nicht blockiert, wenn noch kein Paket bereit
	   ist, sondern einen Fehlercode erzeugt */
	if (deskriptor_nicht_blockieren(session_spiel_socket))
		netzwerk_fehler("Can't switch session socket to non-blocking mode:");

	/* Sockets fuer die Interprozess-Kommunikation erzeugen */
	if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sockpair) < 0)
		netzwerk_fehler("Can't create interprocess sockets:");

	/* Session-Prozess abspalten */
	if ((session_prozess = fork()) < 0)
		netzwerk_fehler("Can't fork session process:");

	/* ist dies der Session-Prozess? */
	if (session_prozess == 0)
	{
		/* die Server-Seite des Kommunikations-Sockets schliessen */
		if (close(sockpair[0]))
			netzwerk_fehler("Can't close other interprocess socket:");

		/* den TCP-Socket des Servers schliessen */
		if (close(server_prolog_socket))
			netzwerk_fehler("Can't close prolog socket:");

		/* den Kommunikations-Socket in den nicht blockierenden
		   Modus schalten */
		if (deskriptor_nicht_blockieren(sockpair[1]))
			netzwerk_fehler(
				"Can't switch interprocess socket to non-blocking mode:");

		/* den Kommunikations-Socket speichern */
		socket_zum_server = sockpair[1];

		/* dies ist der Session-Prozess */
		return NULL;
	}
	else
	{
		int *session;                     /* Deskriptor fuer die
		                                     Kommunikation mit dem
		                                     Session-Prozess */
		ADDRLEN_T spiel_adresse_laenge;   /* Groesse der IP-Adresse */
		struct sockaddr_in spiel_adresse; /* IP-Adresse des UDP-Sockets
		                                     der Session */

		/* IP-Adresse des UDP-Sockets der Session abfragen */
		spiel_adresse_laenge = sizeof spiel_adresse;
		if (getsockname(session_spiel_socket,
			(struct sockaddr *)&spiel_adresse,
			&spiel_adresse_laenge))
			netzwerk_fehler("Can't figure out session socket address:");

		/* die Session-Seite des Kommunikations-Sockets schliessen */
		if (close(sockpair[1]))
			netzwerk_fehler("Can't close other interprocess socket:");

		/* den UDP-Socket der Session schliessen */
		if (close(session_spiel_socket))
			netzwerk_fehler("Can't close session socket:");

		/* den Kommunikations-Socket in den nicht blockierenden
		   Modus schalten */
		if (deskriptor_nicht_blockieren(sockpair[0]))
			netzwerk_fehler(
				"Can't switch interprocess socket to non-blocking mode:");

		/* Speicher fuer den Session-Deskriptor belegen */
		speicher_belegen((void **)&session, 2 * sizeof(int));

		/* den Kommunikations-Socket und die Portnummer speichern */
		session[0] = sockpair[0];
		session[1] = ntohs(spiel_adresse.sin_port);

		/* mitloggen */
		if (log_deskriptor != NULL)
			fprintf(log_deskriptor, "%s: session started on %d/udp\n",
				uhrzeit(), session[1]);

		/* den Session-Deskriptor an den Server-Prozess zurueckgeben */
		return session;
	}
}


/*
** nachricht_senden
**  sendet eine Nachricht an einen bestimmten Prozess
**
** Parameter:
**  session: Deskriptor fuer den Session-Prozess bzw. NULL
**           fuer den Server-Prozess
**  verbindung: Deskriptor, der in der Nachricht enthalten sein
**              soll oder NULL
**  daten_laenge: Laenge des Datenteils
**  daten: Datenteil der Nachricht
*/
void nachricht_senden(void *session, void *verbindung, int daten_laenge,
	void *daten)
{
	int socket;      /* Socket, ueber den die Nachricht gesendet werden soll */
	char *nachricht; /* die Nachricht */
	int offset;      /* relative Position, an der der Datenteil beginnt */
	int laenge;      /* Laenge der gesamten Nachricht */

	if (session == NULL)
		/* Nachricht zum Server senden */
		socket = socket_zum_server;
	else
		/* Nachricht an die angegebene Session senden */
		socket = *(int *)session;

	/* Position des Datenteils berechnen */
	offset = verbindung == NULL ? sizeof(struct nachricht_kopf) :
		sizeof(struct nachricht);

	/* Laenge der gesamten Nachricht berechnen */
	laenge = offset + daten_laenge;

	/* Speicher fuer die Nachricht belegen */
	speicher_belegen((void **)&nachricht, laenge);

	/* Nachrichtenkopf initialisieren */
	((struct nachricht_kopf *)nachricht)->laenge = daten_laenge;
	((struct nachricht_kopf *)nachricht)->verbindung = verbindung != NULL;

	/* Deskriptor mitsenden? */
	if (verbindung != NULL)
	{
		/* Verbindungs-Deskriptor kopieren */
		memcpy(&((struct nachricht *)nachricht)->verbindung.client_adresse,
			&((struct verbindung *)verbindung)->adresse,
			sizeof(struct sockaddr_in));
		((struct nachricht *)nachricht)->verbindung.auf_spiel =
			((struct verbindung *)verbindung)->auf_spiel;
	}

	/* Datenteil kopieren */
	if (daten_laenge)
		memcpy(nachricht + offset, daten, daten_laenge);

	/* Nachricht senden */
	if (send(socket, (void *)nachricht, laenge, 0) != laenge)
		/* bei Fehler Prozess mit oder ohne Meldung beenden */
		if (init_verbindung != NULL)
			exit(1);
		else
			netzwerk_fehler("Can't send interprocess message:");

	/* Deskriptor mitgesandt? */
	if (verbindung != NULL)
		/* dieser Deskriptor ist nicht mehr das Original */
		((struct verbindung *)verbindung)->original = 0;
}


/*
** nachricht_empfangen
**  empfaengt eine Nachricht von einem bestimmten Prozess
**
** Parameter:
**  session: Deskriptor fuer den Session-Prozess bzw. NULL
**           fuer den Server-Prozess
**  verbindung: Zeiger auf einen Deskriptor, der in der Nachricht
**              enthalten ist oder NULL
**  daten_laenge: Zeiger auf Laenge des Datenteils
**  daten: Zeiger auf Datenteil der Nachricht
**
** Rueckgabewert:
**  1, falls keine Nachricht empfangen werden konnte, sonst 0
**
** Seiteneffekte:
**  *verbindung und *typ werden gesetzt
*/
int nachricht_empfangen(void *session, void **verbindung, int *daten_laenge,
	void **daten)
{
	int socket;           /* Socket, ueber den die Nachricht
	                         empfangen werden soll */
	int laenge;           /* Laenge der Nachricht */
	char nachricht[1024]; /* Puffer fuer die Nachricht */
	int offset;           /* relative Position, an der der Datenteil beginnt */

	if (session == NULL)
		/* Nachricht vom Server empfangen */
		socket = socket_zum_server;
	else
		/* Nachricht von der angegebenen Session empfangen */
		socket = *(int *)session;

	/* Nachricht empfangen */
	if ((laenge = recv(socket, (void *)nachricht, sizeof nachricht, 0)) <= 0)
	{
		/* Unterbrechung durch ein Signal oder keine Nachricht vorhanden? */
		if (errno == EINTR || errno == EWOULDBLOCK ||
			errno == EAGAIN || laenge == 0)
			/* keine Nachricht */
			return 1;

		/* bei anderem Fehler Programm mit Fehlermeldung beenden */
		netzwerk_fehler("Can't receive interprocess message:");
	}

	/* Nachricht zu kurz fuer korrekten Kopf? */
	if (laenge < sizeof(struct nachricht_kopf))
		return 1;

	/* Laenge des Datenteils kopieren */
	*daten_laenge = ((struct nachricht_kopf *)nachricht)->laenge;

	/* relative Position des Datenteils berechnen */
	offset = ((struct nachricht_kopf *)nachricht)->verbindung ?
		sizeof(struct nachricht) : sizeof(struct nachricht_kopf);

	/* stimmt die Gesamtlaenge? */
	if (laenge != offset + *daten_laenge)
		return 1;

	/* ist ein Verbindungs-Deskriptor enthalten? */
	if (((struct nachricht_kopf *)nachricht)->verbindung)
	{
		/* Verbindungs-Deskriptor kopieren */
		*verbindung = verbindung_init(&((struct nachricht *)
			nachricht)->verbindung.client_adresse);
		((struct verbindung *)*verbindung)->auf_spiel =
			((struct nachricht *)nachricht)->verbindung.auf_spiel;
		((struct verbindung *)*verbindung)->original = 1;
	}
	else
		*verbindung = NULL;

	/* Laenge des Datenteils nicht 0 */
	if (*daten_laenge)
	{
		/* Datenteil kopieren */
		speicher_belegen(daten, *daten_laenge);
		memcpy(*daten, nachricht + offset, *daten_laenge);
	}

	/* Nachricht empfangen */
	return 0;
}


/*
** nachricht_erwarten
**  wartet auf eine Nachricht von einem bestimmten Prozess
**
** Parameter:
**  session: Deskriptor fuer den Session-Prozess bzw. NULL
**           fuer den Server-Prozess
*/
void nachricht_erwarten(void *session)
{
	fd_set readfds; /* Deskriptoren, auf die gewartet wird */

	/* initialisieren */
	FD_ZERO(&readfds);

	if (session == NULL)
		/* auf Nachricht vom Server warten */
		FD_SET(socket_zum_server, &readfds);
	else
		/* auf Nachricht von der angegebenen Session warten */
		FD_SET(*(int *)session, &readfds);

	/* warten, bei Fehler ausser Unterbrechung durch ein Signal
	   Programm mit Fehlermeldung beenden */
	if (select(max_deskriptor_anzahl(), &readfds, NULL, NULL, NULL) < 0 &&
		errno != EINTR)
		netzwerk_fehler("Can't wait for interprocess message:");
}


/*
** nachricht_oder_verbindung_erwarten
**  wartet, bis fuer den Server-Prozess eine Nachricht von einem
**  der anderen Prozesse oder eine neue Verbindung zu einem
**  Client bereitsteht
**
** Parameter:
**  anzsessions: Anzahl der Sessions
**  sessions: Feld mit den Deskriptoren der Sessions
**
** Rueckgabewert:
**  NULL, bei einer Verbindung;
**  ein Deskriptor fuer die Session bei einer Nachricht
*/
void *nachricht_oder_verbindung_erwarten(int anzsessions, void **sessions)
{
	/* Schleife, bis etwas eintrifft */
	for (;;)
	{
		fd_set readfds; /* Deskriptoren, auf die gewartet wird */
		int i;          /* Index in die Sessionliste */

		/* auf Daten auf dem Prolog-Socket des Servers oder
		   den Sockets zu den Sessions warten */
		FD_ZERO(&readfds);

		FD_SET(server_prolog_socket, &readfds);
		for (i = 0; i < anzsessions; i++)
			FD_SET(*(int *)sessions[i], &readfds);

		if (select(max_deskriptor_anzahl(), &readfds, NULL, NULL, NULL) < 0)
		{
			/* Unterbrechung durch ein Signal? */
			if (errno == EINTR)
				/* weiter warten */
				continue;

			/* bei anderem Fehler Programm mit Fehlermeldung beenden */
			netzwerk_fehler(
			"Can't wait for interprocess message or connection from client:");
		}

		/* Daten auf dem Prolog-Socket? */
		if (FD_ISSET(server_prolog_socket, &readfds))
			/* dann NULL zurueckgeben */
			return NULL;

		/* Daten auf einem der Session-Sockets? */
		for (i = 0; i < anzsessions; i++)
			if (FD_ISSET(*(int *)sessions[i], &readfds))
				/* Deskriptor der Session zurueckgeben */
				return sessions[i];
	}
}


char *name_der_verbindung(void *verbindung)
{
	char *name, *puffer;

	name = inet_ntoa(((struct verbindung *)verbindung)->adresse.sin_addr);

	speicher_belegen((void **)&puffer, strlen(name) + 1);
	strcpy(puffer, name);

	return puffer;
}
