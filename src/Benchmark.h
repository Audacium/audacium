/**********************************************************************

  Audacium: A Digital Audio Editor

  Benchmark.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_BENCHMARK__
#define __AUDACITY_BENCHMARK__

class wxWindow;
class AudaciumProject;

AUDACITY_DLL_API
void RunBenchmark( wxWindow *parent, AudaciumProject &project );

#endif // define __AUDACITY_BENCHMARK__
