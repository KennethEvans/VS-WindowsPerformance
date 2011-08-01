using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics.Eventing.Reader;

namespace WindowsPerformanceReview {
    class Program {
        private static readonly String OUTPUT_FILE = "WindowsPerformanceReview.txt";
        private static readonly String BOOT_CSV_FILE = "WindowsPerformanceReview.BootTime.csv";
        private static Boolean doBootCsv = true;
        private static Boolean bootCsvFileWritten = false;

        /////////////////////////////////////////////////////////////////////
        // Main
        /////////////////////////////////////////////////////////////////////
        static void Main(String[] args) {
            // Create a writer and open the file
            TextWriter swOut = new StreamWriter(OUTPUT_FILE);
            // Write the header
            swOut.WriteLine("WindowsPerformanceReview " + DateTime.Now);
            swOut.WriteLine();

#if false
            // Read from the Application log
            swOut.WriteLine(DiagnosticsUtils.readLog("Application", 20));
#endif
#if false
            // Try to read an Applications and Services log.
            // Doesn't work unless one of the standard logs
            swOut.WriteLine(
                DiagnosticsUtils.readLog("Microsoft-Windows-Diagnostics-Performance/Operational", 20));
#endif
#if false
            // Query an Applications and Services log
            //String queryString = "*[System/Level=2]"; // XPATH Query
            String queryString = "*"; // XPATH Query
            String eventLog = "Microsoft-Windows-Diagnostics-Performance/Operational";
            PathType pathType = PathType.LogName;
            swOut.WriteLine(DiagnosticsUtils.queryLogFileXml(queryString, pathType, eventLog));
#endif
#if true
            // Get the diagnostics event provider
            swOut.WriteLine(DiagnosticsUtils.getDiagnosticsEventProvider());
#endif
#if false
            // Get the boot times
            swOut.WriteLine("Boot Times");
            List<String[]> bootTimes = DiagnosticsUtils.getBootTimes();
            if (bootTimes == null) {
                swOut.WriteLine(Utils.errMsg("Could not get boot times"));
            } else {
                foreach (String[] times in bootTimes) {
                    swOut.WriteLine(DiagnosticsUtils.SEPARATOR_LINE);
                    swOut.WriteLine("BootStartTime: " + times[0]);
                    swOut.WriteLine("BootTime: " + times[1]);
                    swOut.WriteLine("MainPathBootTime: " + times[2]);
                    swOut.WriteLine("BootPostBootTime: " + times[3]);
                }
                swOut.WriteLine(DiagnosticsUtils.SEPARATOR_LINE);
                swOut.WriteLine("Number of data points: " + bootTimes.Count);
            }
            swOut.WriteLine();
#endif
#if false
            // Make a CSV file
            swOut.WriteLine("Making Boot Times CSV file " + BOOT_CSV_FILE);
            if (doBootCsv) {
                String res = DiagnosticsUtils.createBootTimesCsvFile(BOOT_CSV_FILE);
                if (!res.Equals("OK")) {
                    swOut.WriteLine(res);
                } else {
                    bootCsvFileWritten = true;
                }
            }
            swOut.WriteLine();
#endif
#if false
            // Test
            swOut.WriteLine(Test.test());
#endif

            // Clean up
            String cwd = Directory.GetCurrentDirectory();
            swOut.WriteLine("Output file is " + cwd + Path.DirectorySeparatorChar + OUTPUT_FILE);
            if (bootCsvFileWritten) {
                swOut.WriteLine("Boot CSV file is " + cwd + Path.DirectorySeparatorChar + BOOT_CSV_FILE);
            }
            // Close the output stream
            if (swOut != null) {
                swOut.Close();
            }
        }

    }
}
