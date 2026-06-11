/*
 * Copyright 2006, CERMA, Nantes, France.
 */

#ifndef _TC_SOLENE_PATHS_H_
#define _TC_SOLENE_PATHS_H_

/*
 * les directives de pre-compilation suivantes sont plateformes dependantes
 */
// modif laurent malys 22-01-2010

#ifdef LINUX
/* chemins a la facon UNIX */
#define TMP_SOLENE "/home/laurent/solene/tmp"
#define OK_SOLENE "/home/laurent/solene/tmp/OK_SOLENE"

#else
/* chemins a la facon Win32 */
#define TMP_SOLENE "C:\\Solene\\Private\\Temp\\"
#define OK_SOLENE "C:\\Solene\\Private\\Temp\\OK_SOLENE"

#endif

#endif
