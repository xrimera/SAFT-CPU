USCT Addsig2VoL Ellipsoide backprojection/Synthetic aperture imaging Imaging Core 
Karlsruhe Institute of Technology, Michael Zapf, 2016

About:
-------
Makes an SAFT 3D image reconstruction. Fast. Limited parameter checks, so beware of wrong parameter handover!

Some paper
https://www.academia.edu/18822458/_title_High_throughput_SAFT_for_an_experimental_USCT_system_as_MATLAB_implementation_with_use_of_SIMD_CPU_instructions_title_


license
----------
FOSS software under GPLv2.1

(contact me if that is a problem)


Features:
---------
- SSE instruction usage, effective register usage, data locality -> fast
- multithreading capable (PThreads needed)
- portable to Windows/Linux
- ia32/ia64 support

- Performance for P4-2GHz: 42 MVoxel/s, 1GVoxel/s with octacore. 
- one order faster then c-code, 40 times faster then Matlab code. 


Requirements:
-------------
- CPU with SSE1 support (since P3 Katmai)
- Matlab 6.x up to 7.3 tested (Octave not tested, but should work)
- supported environments: Win32, Win64, Linux32 & Linux64
 

Usage: 
------
-	addsig2vol; 
 -> gives you parameter help
 

-	addsig2vol(8); 
 -> sets persistent the used core number, if not autoset by benchmark



Build Notes in the readme in the subdirectory
