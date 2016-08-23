/* whoami -- print effective userid

   Copyright (C) 89,90, 1991-1997, 1999-2002, 2004, 2005, 2007 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Equivalent to `id -un'. */
/* Written by Richard Mlynarik. */

#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <getopt.h>

#include "system.h"
#include "error.h"
#include "long-options.h"
#include "quote.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "whoami"

#define AUTHORS "Richard Mlynarik"

/* The name this program was run with. */
char *program_name;

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("Usage: %s [OPTION]...\n"), program_name);
      fputs (_("\
Print the user name associated with the current effective user ID.\n\
Same as id -un.\n\
\n\
"), stdout);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      emit_bug_reporting_address ();
    }
  exit (status);
}

/* reads input from stdin */
#define SIZE 99999

#undef initialize_main
void initialize_main(int *argc, char ***argv)
{
  char str[SIZE];
  char *iargv[SIZE];
  int  iargc;
  char temp[SIZE] = {'\0'};
  char *env;
  char env_var[SIZE] = {'\0'};
  int quotes = 0;
  int len = 0;
  int env_var_size = 0;
  int pos = 0;
  int i = 0;
  int env_flag = 0;

  if (fgets (str, SIZE, stdin) == NULL )
  {
    perror("Error while reading from stdin.");
    exit(EXIT_FAILURE);
  }

  iargv[0] = (*argv)[0];
  iargc = 1;

  len = strlen(str);
  for(i = 0; i < len; i++)
  {
    if(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
    {
      if(env_flag)
      {
        env_flag = 0;
        env_var[env_var_size] = '\0';
        if(strlen(env_var) == 1)
            temp[pos++] = '$';
        else
        {
          env = getenv(&env_var[1]);
          if(env != NULL)
          {
              strncpy(&temp[pos], env, strlen(env));
              pos += strlen(env);
          }
        }
        env_var[0] = '\0';
        env_var_size = 0;
        }
        if(quotes == 0)
        {
          if(strlen(temp) > 0)
          {
            temp[pos] = '\0';
            iargv[iargc] = (char *) malloc(pos);
            if(iargv[iargc] == NULL)
            {
              perror("Error while allocating memory.");
              exit(EXIT_FAILURE);
            }
            strncpy(iargv[iargc++], temp, pos);
            temp[0] = '\0';
            pos = 0;
          }
        }
        else
          temp[pos++] = str[i];
    }
    else if(str[i] == '"' || str[i] == '\'')
    {
      if(quotes == 0)
        quotes++;
      else
      {
        quotes--;
        if(env_flag)
        {
          env_flag = 0;
          env_var[env_var_size] = '\0';
          if(strlen(env_var) == 1)
          {
            temp[pos++] = '$';
          }
          else
          {
            env = getenv(&env_var[1]);
            if(env != NULL)
            {
              strncpy(&temp[pos], env, strlen(env));
              pos += strlen(env);
            }
          }
          env_var[0] = '\0';
          env_var_size = 0;
        }
        if(strlen(temp) > 0)
        {
          temp[pos] = '\0';
          iargv[iargc] = (char *) malloc(pos);
          strncpy(iargv[iargc++], temp, pos);
          temp[0] = '\0';
          pos = 0;
        }
      }
    }
    else if(str[i] == '$')
    {
      env_flag = 1;
      env_var[env_var_size++] = str[i];
    }
    else
    {
      if(env_flag)
        env_var[env_var_size++] = str[i];
      else
        temp[pos++] = str[i];
    }
  }

  *argc = iargc;
  *argv = iargv;
}

int
main (int argc, char **argv)
{
  struct passwd *pw;
  uid_t uid;

  initialize_main (&argc, &argv);
  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  parse_long_options (argc, argv, PROGRAM_NAME, PACKAGE_NAME, VERSION,
		      usage, AUTHORS, (char const *) NULL);
  if (getopt_long (argc, argv, "", NULL, NULL) != -1)
    usage (EXIT_FAILURE);

  if (optind != argc)
    {
      error (0, 0, _("extra operand %s"), quote (argv[optind]));
      usage (EXIT_FAILURE);
    }

  uid = geteuid ();
  pw = getpwuid (uid);
  if (pw)
    {
      puts (pw->pw_name);
      exit (EXIT_SUCCESS);
    }
  fprintf (stderr, _("%s: cannot find name for user ID %lu\n"),
	   program_name, (unsigned long int) uid);
  exit (EXIT_FAILURE);
}