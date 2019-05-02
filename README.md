# tbx
C++ toolbox library

This library is split into different modules with different dependencies

tbx\
Extensions to standard library are here.
This is the root namespace.  It is dependent only upon the C++17 standard library.  I hope to reduce to being partially or mostly functional for C++14 as well.

tbx\crt\
Extensions to the C language runtime are here.

tbx\wapi\
Windows API extensions and dependencies are here.

tbx\mfc\
Finally, everything that augments and relies upon ATL/MFC is located here.  To compile this library you will need to have a professional license for MSVC with ATL/MFC support installed.

Unit Testing / BDD:
Each library also has a console app that uses catch2.h to implement various unit tests:

tbx\test\test_tbx\
Tests core tbx library

tbx\test\test_wapi\
Tests the tbx\wapi library

tbx\test\test_mfc\
Tests the tbx\mfc library
