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
** Datei: system.c
**
** Kommentar:
**  Globale systemnahe Kompatibiltaetsroutinen;
**  Signal-Handling, Timing etc.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#ifdef sun
#include <sys/filio.h>
#endif
#ifdef sinix
#include <sys/filio.h>
#endif

#include "system.h"
#include "speicher.h"

static char sccsid[] = "@(#)system.c	3.9 12/3/01";


/* Systemvariable: letzter aufgetretener Fehlercode oder 0 */
extern int errno;


/* Struktur, in der der Timerzustand gerettet werden kann */
struct timer_zustand
{
	void (*handler)(int signum);
	struct itimerval wert;
};


/*
** leer_handler
**  faengt ein Signal auf
**
** Parameter:
**  signum: Dummy-Parameter
*/
static void leer_handler(int signum)
{
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** handle_signal
**  simuliert das Verhalten des BSD-signal mit sigaction
**
** Parameter:
**  signum: zu behandelndes Signal
**  handler: Signal-Handler
**
** Rueckgabewert:
**  vorheriger Signal-Handler
*/
void (*handle_signal(int signum, void (*handler)(int signum)))(int signum)
{
	struct sigaction neue_sa; /* neues Verhalten bei diesem Signal */
	struct sigaction alte_sa; /* vorheriges Verhalten bei diesem Signal */

	neue_sa.sa_handler = handler;  /* Signal-Handler setzen */
	sigemptyset(&neue_sa.sa_mask); /* keine zusaetzlichen Signale
	                                 im Handler sperren */
	neue_sa.sa_flags = 0;          /* keine zusaetzlichen Flags */

	/* neues Verhalten aktivieren, vorheriges auslesen */
	if (sigaction(signum, &neue_sa, &alte_sa) == -1)
		/* bei Fehler -1 zurueckgeben */
		return (void (*)(int))-1;

	/* vorherigen Signal-Handler zurueckgeben */
	return alte_sa.sa_handler;
}


/*
** signal_blockiert
**  fragt ab, ob ein bestimmtes Signal blockiert ist
**
** Parameter:
**  signum: abzufragendes Signal
**
** Rueckgabewert:
**  1, falls das Signal blockiert ist; sonst 0
*/
int signal_blockiert(int signum)
{
#ifdef SIG_SETMASK
	sigset_t maske; /* blockierte Signale */

	sigprocmask(SIG_SETMASK, NULL, &maske); /* Maske abfragen */
	return sigismember(&maske, signum);
#else
	return (sigblock(0) & sigmask(signum)) != 0;
#endif
}


/*
** signal_blockieren
**  blockiert ein Signal
**
** Parameter:
**  signum: zu blockierendes Signal
*/
void signal_blockieren(int signum)
{
#ifdef SIG_BLOCK
	sigset_t maske; /* Maske fuer zu blockierendes Signal */

	sigemptyset(&maske); /* Maske loeschen */
	sigaddset(&maske, signum); /* Signal zu Maske hinzufuegen */
	sigprocmask(SIG_BLOCK, &maske, NULL); /* Signal blockieren */
#else
	sigblock(sigmask(signum)); /* Signal blockieren */
#endif
}


/*
** signal_freigeben
**  gibt ein blockiertes Signal wieder frei
**
** Parameter:
**  signum: freizugebendes Signal
*/
void signal_freigeben(int signum)
{
#ifdef SIG_UNBLOCK
	sigset_t maske; /* Maske fuer freizugebendes Signal */

	sigemptyset(&maske); /* Maske loeschen */
	sigaddset(&maske, signum); /* Signal zu Maske hinzufuegen */
	sigprocmask(SIG_UNBLOCK, &maske, NULL); /* Signal freigeben */
#else
	sigsetmask(sigblock(0) & ~sigmask(signum)); /* Signal freigeben */
#endif
}


/*
** max_deskriptor_anzahl
**  stellt die maximale Anzahl der Deskriptoren dieses Prozesses fest
**
** Rueckgabewert:
**  maximale Anzahl der Deskriptoren
*/
int max_deskriptor_anzahl(void)
{
	return FD_SETSIZE;
}


/*
** deskriptor_nicht_blockieren
**  schaltet einen Deskriptor in den nicht blockierenden Modus
**
** Parameter:
**  deskriptor: Deskriptor, der in den nicht blockierenden Modus
**              geschaltet werden soll
**
** Rueckgabewert:
**  0 bei Erfolg, -1 bei Fehler
*/
int deskriptor_nicht_blockieren(int deskriptor)
{
	int eins = 1; /* Konstante fuer den ioctl-Aufruf */

	return ioctl(deskriptor, FIONBIO, &eins);
}


/*
** puffer_lesen
**  liest durch einen Deskriptor einen Puffer, falls moeglich,
**  komplett voll
**
** Parameter:
**  deskriptor: Deskriptor, durch den gelesen werden soll
**  puffer_laenge: Groesse des Puffers
**  puffer: der Puffer
**
** Rueckgabewert:
**  -1 fuer Fehler, sonst die Anzahl der gelesenen Bytes; puffer_laenge,
**  falls kein EOT gelesen wurde
*/
int puffer_lesen(int deskriptor, int puffer_laenge, void *puffer)
{
	int laenge_gesamt; /* Anzahl der bisher gelesenen Bytes */

	/* Schleife, solange noch Platz im Puffer ist */
	for (laenge_gesamt = 0; puffer_laenge;)
	{
		int laenge_neu; /* Anzahl der aktuell gelesenen Bytes oder -1 */

		/* maximal puffer_laenge Bytes lesen */
		if ((laenge_neu = read(deskriptor, puffer, puffer_laenge)) < 0)
		{
			/* Fehler EINTR (Interrupted System Call) ignorieren */
			if (errno == EINTR)
				continue;

			/* bei anderem Fehler Wert < 0 zurueckgeben */
			return laenge_neu;
		}

		/* EOT, d.h. keine Bytes gelesen? */
		if (!laenge_neu)
			break;

		puffer = (char *)puffer + laenge_neu; /* Pufferzeiger erhoehen */
		puffer_laenge -= laenge_neu; /* verbleibende Puffergroesse
		                                vermindern */
		laenge_gesamt += laenge_neu; /* Anzahl gelesener Bytes erhoehen */
	}

	/* Anzahl insgesamt gelesener Bytes zurueckgeben */
	return laenge_gesamt;
}


/*
** puffer_schreiben
**  schreibt einen Puffer, falls moeglich, komplett durch einen Deskriptor
**
** Parameter:
**  deskriptor: Deskriptor, durch den geschrieben werden soll
**  puffer_laenge: Groesse des Puffers
**  puffer: der Puffer
**
** Rueckgabewert:
**  -1 fuer Fehler, sonst die Anzahl der geschriebenen Bytes; puffer_laenge,
**  falls write nicht 0 zurueckgegeben hat (sollte nicht passieren)
*/
int puffer_schreiben(int deskriptor, int puffer_laenge, void *puffer)
{
	int laenge_gesamt; /* Anzahl der bisher geschriebenen Bytes */

	/* Schleife, solange noch Bytes im Puffer sind */
	for (laenge_gesamt = 0; puffer_laenge;)
	{
		int laenge_neu; /* Anzahl der aktuell geschriebenen Bytes oder -1 */

		/* maximal puffer_laenge Bytes schreiben */
		if ((laenge_neu = write(deskriptor, puffer, puffer_laenge)) < 0)
		{
			/* Fehler EINTR (Interrupted System Call) ignorieren */
			if (errno == EINTR)
				continue;

			/* bei anderem Fehler Wert < 0 zurueckgeben */
			return laenge_neu;
		}

		/* keine Bytes geschrieben? d.h. kein Schreiben moeglich,
		   aber kein Fehler */
		if (!laenge_neu)
			break;

		puffer = (char *)puffer + laenge_neu; /* Pufferzeiger erhoehen */
		puffer_laenge -= laenge_neu; /* Anzahl verbleibender Bytes
		                                vermindern */
		laenge_gesamt += laenge_neu; /* Anzahl geschriebener Bytes
		                                erhoehen */
	}

	/* Anzahl insgesamt geschriebener Bytes zurueckgeben */
	return laenge_gesamt;
}


/*
** timer_starten
**  startet den Timer; nach der gewaehlten Zeit wird ein SIGALRM ausgeloest
**
** Parameter:
**  ms: Zeit in Millisekunden
*/
void timer_starten(int ms)
{
	struct itimerval neuer_wert; /* neuer Timerzustand */

	/* Timer nach Ablauf nicht automatisch neu starten */
	neuer_wert.it_interval.tv_sec = 0;
	neuer_wert.it_interval.tv_usec = 0;

	/* Timer auf ms Millisekunden setzen */
	neuer_wert.it_value.tv_sec = ms / 1000;
	neuer_wert.it_value.tv_usec = (ms % 1000) * 1000;

	/* Timer setzen */
	setitimer(ITIMER_REAL, &neuer_wert, NULL);
}


/*
** timer_stoppen
**  stoppt den Timer; danach wird kein SIGALRM mehr ausgeloest
*/
void timer_stoppen(void)
{
	struct itimerval neuer_wert; /* neuer Timerzustand */

	/* Timer nach Ablauf nicht automatisch neu starten */
	neuer_wert.it_interval.tv_sec = 0;
	neuer_wert.it_interval.tv_usec = 0;

	/* Timer auf "abgelaufen" setzen */
	neuer_wert.it_value.tv_sec = 0;
	neuer_wert.it_value.tv_usec = 0;

	/* Timer setzen */
	setitimer(ITIMER_REAL, &neuer_wert, NULL);
}


/*
** timer_restzeit
**  fragt die verbleibende Zeit bis zum Ablauf des Timers ab
**
** Rueckgabewert:
**  verbleibende Zeit in Millisekunden
*/
int timer_restzeit(void)
{
	struct itimerval wert; /* Timerzustand */

	/* Timerzustand abfragen */
	getitimer(ITIMER_REAL, &wert);

	/* verbleibende Zeit umrechnen und zurueckgeben */
	return wert.it_value.tv_sec * 1000 +
		(wert.it_value.tv_usec + 999) / 1000;
}


/*
** timer_abwarten
**  wartet, bis der Timer abgelaufen ist
*/
void timer_abwarten(void)
{
#ifdef SIG_SETMASK
	sigset_t alte_maske;     /* alte Signal-Maske */
	sigset_t maske;          /* Maske mit Timer-Signal */
#else
	int alte_maske;          /* alte Signal-Maske */
#endif

	void (*alter_handler)(int); /* alter Signal-Handler */
	struct itimerval wert;   /* Timerzustand */

	/* Timer-Signal blockieren und alte Maske retten,
	   siehe signal_blockieren */
#ifdef SIG_SETMASK
	sigemptyset(&maske);
	sigaddset(&maske, SIGALRM);
	sigprocmask(SIG_BLOCK, &maske, &alte_maske);
#else
	alte_maske = sigblock(sigmask(SIGALRM));
#endif

	/* Dummy-Handler fuer das Timer-Signal installieren */
	alter_handler = signal(SIGALRM, leer_handler);

	for (;;)
	{
		/* Timerzustand abfragen */
		getitimer(ITIMER_REAL, &wert);

		/* Timer bereits abgelaufen? */
		if (!wert.it_value.tv_sec && !wert.it_value.tv_usec)
			break;

		/* Timer-Signal temporaer wieder freigeben und auf
		   Ablauf des Timers warten */
#ifdef SIG_SETMASK
		sigsuspend(&alte_maske);
#else
		sigpause(alte_maske);
#endif
	}

	/* alten Signal-Handler wieder zurueckholen */
	signal(SIGALRM, alter_handler);

	/* Timer-Signal wieder freigeben */
#ifdef SIG_SETMASK
	sigprocmask(SIG_SETMASK, &alte_maske, NULL);
#else
	maske = sigsetmask(alte_maske);
#endif
}


/*
** timer_retten
**  fragt den aktuellen Timerzustand ab und haelt den Timer an
**
** Rueckgabewert:
**  Zeiger auf eine Struktur, die den Timerzustand enthaelt
**
** Seiteneffekte:
**  fuer die Struktur wird Speicher belegt
*/
void *timer_retten(void)
{
	struct timer_zustand *zustand; /* vorheriger Timerzustand und
	                                  Signal-Handler */
	struct itimerval neuer_wert;   /* neuer Timerzustand */

	/* Speicher fuer Timerzustand und Signal-Handler belegen */
	speicher_belegen((void **)&zustand, sizeof(struct timer_zustand));

	/* Timer nach Ablauf nicht automatisch neu starten */
	neuer_wert.it_interval.tv_sec = 0;
	neuer_wert.it_interval.tv_usec = 0;

	/* Timer auf "abgelaufen" setzen */
	neuer_wert.it_value.tv_sec = 0;
	neuer_wert.it_value.tv_usec = 0;

	/* Timer abfragen und setzen */
	setitimer(ITIMER_REAL, &neuer_wert, &zustand->wert);

	/* Signal-Handler merken und loeschen */
	zustand->handler = signal(SIGALRM, SIG_IGN);

	/* alten Zustand als typenlosen Zeiger zurueckgeben */
	return (void *)zustand;
}


/*
** timer_restaurieren
**  laesst den Timer ab dem geretteten Zustand weiterlaufen
**
** Parameter:
**  zustand: Zeiger auf Struktur mit dem geretteten Zustand
**
** Seiteneffekte:
**  Speicher, auf den zustand zeigt, wird freigegeben
*/
void timer_restaurieren(void *zustand)
{
	/* Timer stoppen, damit nicht der Signal-Handler unbeabsichtigt
	   aufgerufen wird */
	timer_stoppen();

	/* den geretteten Signal-Handler restaurieren, bevor der Timer neu
	   gestartet wird */
	signal(SIGALRM, ((struct timer_zustand *)zustand)->handler);

	/* Timer mit den geretteten Parametern neu starten */
	setitimer(ITIMER_REAL, &((struct timer_zustand *)zustand)->wert, NULL);

	/* Speicher wieder freigeben */
	speicher_freigeben((void **)&zustand);
}


/*
** uhrzeit
**  gibt die aktuelle Uhrzeit als String in der Form
**  Sat Jan  1 00:00:00 1994 zurueck
**
** Rueckgabewert:
**  String, der die aktuelle Uhrzeit enthaelt
**
** Seiteneffekte:
**  der Rueckgabewert ist ein konstanter Zeiger auf eine static-Variable;
**  bei mehrmaligem Aufruf wird der alte Wert ueberschrieben
*/
char *uhrzeit(void)
{
	time_t zeit;   /* Uhrzeit in Sekunden seit 1.1.1970 */
	char *zeitstr; /* Uhrzeit als String */

	/* Uhrzeit abfragen */
	time(&zeit);

	/* Uhrzeit in String umwandeln */
	zeitstr = ctime(&zeit);

	/* newline am Stringende entfernen */
	if (zeitstr[0] != 0 && zeitstr[strlen(zeitstr) - 1] == '\n')
		zeitstr[strlen(zeitstr) - 1] = 0;

	return zeitstr;
}


/*
** benutzer_name
**  gibt den Namen des Benutzers zurueck
**
** Rueckgabewert:
**  String, der den Namen des Benutzers enthaelt, oder NULL, falls unbekannt
**
** Seiteneffekte:
**  fuer den String wird Speicher belegt, der durch Aufruf von
**  speicher_freigeben wieder freigegeben werden muss
*/
char *benutzer_name(void)
{
	char *benutzer;                /* ID des Benutzers */
	struct passwd *passwd_eintrag; /* Passwort-Eintrag des Benutzers */
	void *puffer;                  /* Puffer fuer den Benutzernamen */
	int laenge;                    /* Laenge des Benutzernamens */
	char *komma;                   /* Position des ersten Kommas */

	/* ID des Benutzers abfragen */
	if ((benutzer = getlogin()) == NULL)
		/* falls nicht moeglich, Passwort-Eintrag ueber numerische ID suchen */
		passwd_eintrag = getpwuid(getuid());
	else
		/* sonst Passwort-Eintrag des Benutzers normal abfragen */
		passwd_eintrag = getpwnam(benutzer);

	if (passwd_eintrag == NULL)
		/* falls Passwort-Eintrag nicht gefunden, NULL zurueckgeben */
		return NULL;

	/* Laenge des Benutzernamens feststellen */
	laenge = strlen(passwd_eintrag->pw_gecos);

	/* alles ab einem Komma ignorieren */
	if ((komma = strchr(passwd_eintrag->pw_gecos, ',')) != NULL)
		laenge = komma - passwd_eintrag->pw_gecos;

	/* hat der Benutzername die Laenge 0? */
	if (laenge == 0)
		return NULL;

	/* Speicher fuer Benutzernamen belegen und kopieren */
	speicher_belegen(&puffer, laenge + 1);
	strncpy(puffer, passwd_eintrag->pw_gecos, laenge);
	((char *)puffer)[laenge] = 0;

	/* Benutzernamen zurueckgeben */
	return puffer;
}


/*
** fehler_text
**  ermittelt den zur Systemvariable errno passenden Fehlertext
**
** Rueckgabewert:
**  Zeiger in das Feld sys_errlist oder in eine lokale Variable, die
**  den Fehler als Nummer enthaelt (falls kein Standardtext vorhanden ist)
**
** Seiteneffekte:
**  der zuletzt zurueckgegebene Zeiger wird evtl. ungueltig
*/
char *fehler_text(void)
{
#ifdef NEED_ERRLIST /* nicht in <errno.h>? */
	extern char *sys_errlist[]; /* Liste der Standard-Fehlertexte */
	extern int sys_nerr;        /* Anzahl der Eintraege in sys_errlist */
#endif

	/* falls errno kein gueltiger Index fuer sys_errlist ist, nur die
	   Nummer als String zurueckgeben */
	if (errno < 0 || errno >= sys_nerr)
	{
		static char text[5][20]; /* mehrere Puffer fuer Fehlernummer */
		static int current = 0;

		current = (current + 1) % (sizeof text / sizeof *text);

		sprintf(text[current], "Error #%d", errno);

		/* Zeiger auf Puffer zurueckgeben; Achtung: dieser Zeiger
		   ist nach ein paar Aufrufen wieder derselbe */
		return text[current];
	}

	/* Standard-Fehlertext zurueckgeben */
	return (char *)sys_errlist[errno];
}
