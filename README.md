# VS-WindowsPerformance

Windows Performance Viewer is a C# application that accesses the Windows event logs and presents details of the performance in convenient ways. It was originally used to see if boot time was increasing over time.

The logs used in this application are those provided by Microsoft-Windows-Diagnostics-Performance. You will probably have to run it as administrator to get access to the diagnostic logs. The program does not change anything, just reads the logs.

See https://rawgit.com/KennethEvans/VS-WindowsPerformance/master/WindowsPerformanceViewer/Help/Overview.html

See https://kennethevans.github.io/index.html#WindowsPerformanceViewer

There is also a C# console project, Windows Performance Review, in the Solution.

**Installation**

If you are installing from a download, just unzip the files into a directory somewhere convenient. Then run it from there. If you are installing from a build, copy these files and directories from the bin/Release directory to a convenient directory.

* WindowsPerformanceViewer.exe
* Help

To uninstall, just delete these files. 

**More Information**

More information and FAQ are at https://kennethevans.github.io as well as more projects from the same author.

Licensed under the MIT license. (See: https://en.wikipedia.org/wiki/MIT_License)