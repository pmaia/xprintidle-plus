/*

This program prints the "idle time" of the user to stdout.  The "idle
time" is the number of milliseconds since input was received on any
input device.  If unsuccessful, the program prints a message to stderr
and exits with a non-zero exit code.

Copyright (c) 2005, 2008 Magnus Henoch <henoch@dtek.chalmers.se>
Copyright (c) 2006, 2007 by Danny Kukawka
                         <dkukawka@suse.de>, <danny.kukawka@web.de>
Copyright (c) 2008 Eivind Magnus Hvidevold <hvidevold@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

The function workaroundCreepyXServer was adapted from kpowersave-0.7.3 by
Eivind Magnus Hvidevold <hvidevold@gmail.com>. kpowersave is licensed under
the GNU GPL, version 2 _only_.

*/

#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/scrnsaver.h>
#include <stdio.h>
#include <unistd.h>

int main_old(int argc, char *argv[]);
void usage(char *name);
unsigned long workaroundCreepyXServer(Display *dpy, unsigned long _idleTime );

int main(int argc, char *argv[]){
	while(1) {
		main_old(1, NULL);
		sleep(1);
	}

	return 0;
}

int main_old(int argc, char *argv[])
{
  printf("main: 1\n");
  XScreenSaverInfo ssi;
  Display *dpy;
  int event_basep, error_basep;
  printf("main: 2\n");
  if (argc != 1) {
    printf("main: 3\n");
    usage(argv[0]);
    printf("main: 4\n");
    return 1;
  }
  printf("main: 5\n");
  dpy = XOpenDisplay(NULL);
  printf("main: 6\n");
  if (dpy == NULL) {
    printf("main: 7\n");
    fprintf(stderr, "couldn't open display\n");
    printf("main: 8\n");
    return 1;
  }
  printf("main: 9\n");
  
  if (!XScreenSaverQueryExtension(dpy, &event_basep, &error_basep)) {
    printf("main: 10\n");
    fprintf(stderr, "screen saver extension not supported\n");
    printf("main: 11\n");
    return 1;
  }
  printf("main: 12\n");
  
  if (!XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), &ssi)) {
    printf("main: 13\n");
    fprintf(stderr, "couldn't query screen saver info\n");
    printf("main: 14\n");
    return 1;
  }
  printf("main: 15\n");
  
  printf("%lu\n", workaroundCreepyXServer(dpy, ssi.idle));
  printf("main: 16\n");
  
  XCloseDisplay(dpy);
  printf("main: 18\n");
  return 0;
}

void usage(char *name)
{
  fprintf(stderr,
	  "Usage:\n"
	  "%s\n"
	  "That is, no command line arguments.  The user's idle time\n"
	  "in milliseconds is printed on stdout.\n",
	  name);
}

/*!
 * This function works around an XServer idleTime bug in the
 * XScreenSaverExtension if dpms is running. In this case the current
 * dpms-state time is always subtracted from the current idletime.
 * This means: XScreenSaverInfo->idle is not the time since the last
 * user activity, as descriped in the header file of the extension.
 * This result in SUSE bug # and sf.net bug #. The bug in the XServer itself
 * is reported at https://bugs.freedesktop.org/buglist.cgi?quicksearch=6439.
 *
 * Workaround: Check if if XServer is in a dpms state, check the 
 *             current timeout for this state and add this value to 
 *             the current idle time and return.
 *
 * \param _idleTime a unsigned long value with the current idletime from
 *                  XScreenSaverInfo->idle
 * \return a unsigned long with the corrected idletime
 */
unsigned long workaroundCreepyXServer(Display *dpy, unsigned long _idleTime ){
  printf("workaroundCreepyXServer: 1\n");
  int dummy;
  CARD16 standby, suspend, off;
  CARD16 state;
  BOOL onoff;
  printf("workaroundCreepyXServer: 2\n");

  if (DPMSQueryExtension(dpy, &dummy, &dummy)) {
  printf("workaroundCreepyXServer: 3\n");
    if (DPMSCapable(dpy)) {
  printf("workaroundCreepyXServer: 4\n");
      DPMSGetTimeouts(dpy, &standby, &suspend, &off);
  printf("workaroundCreepyXServer: 5\n");
      DPMSInfo(dpy, &state, &onoff);
    printf("workaroundCreepyXServer: 6\n");

      if (onoff) {
  printf("workaroundCreepyXServer: 7\n");
        switch (state) {
          case DPMSModeStandby:
  printf("workaroundCreepyXServer: 8\n");
            /* this check is a littlebit paranoid, but be sure */
            if (_idleTime < (unsigned) (standby * 1000)){
		  printf("workaroundCreepyXServer: 9\n");
              _idleTime += (standby * 1000);
	    }
            break;
          case DPMSModeSuspend:
            printf("workaroundCreepyXServer: 10\n");
            if (_idleTime < (unsigned) ((suspend + standby) * 1000)) {
		  printf("workaroundCreepyXServer: 11\n");
              _idleTime += ((suspend + standby) * 1000);
            }
            break;
          case DPMSModeOff:
		  printf("workaroundCreepyXServer: 12\n");
            if (_idleTime < (unsigned) ((off + suspend + standby) * 1000)){
		  printf("workaroundCreepyXServer: 13\n");
              _idleTime += ((off + suspend + standby) * 1000);
	}
            break;
          case DPMSModeOn:   printf("workaroundCreepyXServer: 14\n");
          default:
            break;
        }
      }
	  printf("workaroundCreepyXServer: 15\n");
    } 
	  printf("workaroundCreepyXServer: 16\n");
  }

  return _idleTime;
}
