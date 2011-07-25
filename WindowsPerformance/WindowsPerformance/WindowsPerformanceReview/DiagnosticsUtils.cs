using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Eventing.Reader;
using System.Globalization;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using WindowsPerformanceReview;

class DiagnosticsUtils {
    public static readonly String LF = "\r\n";
    public static readonly String[] BOOT_DATA_NAMES = {
        "BootStartTime",
        "BootTime",
        "MainPathBootTime",
        "BootPostBootTime",

        "BootTsVersion",
        "BootEndTime",
        "SystemBootInstance",
        "UserBootInstance",
        "BootKernelInitTime",
        "BootDriverInitTime",
        "BootDevicesInitTime",
        "BootPrefetchInitTime",
        "BootPrefetchBytes",
        "BootAutoChkTime",
        "BootSmssInitTime",
        "BootCriticalServicesInitTime",
        "BootUserProfileProcessingTime",
        "BootMachineProfileProcessingTime",
        "BootExplorerInitTime",
        "BootNumStartupApps",
        "BootIsRebootAfterInstall",
        "BootRootCauseStepImprovementBits",
        "BootRootCauseGradualImprovementBits",
        "BootRootCauseStepDegradationBits",
        "BootRootCauseGradualDegradationBits",
        "BootIsDegradation",
        "BootIsStepDegradation",
        "BootIsGradualDegradation",
        "BootImprovementDelta",
        "BootDegradationDelta",
        "BootIsRootCauseIdentified",
        "OSLoaderDuration",
        "BootPNPInitStartTimeMS",
        "BootPNPInitDuration",
        "OtherKernelInitDuration",
        "SystemPNPInitStartTimeMS",
        "SystemPNPInitDuration",
        "SessionInitStartTimeMS",
        "Session0InitDuration",
        "Session1InitDuration",
        "SessionInitOtherDuration",
        "WinLogonStartTimeMS",
        "OtherLogonInitActivityDuration",
        "UserLogonWaitDuration",
    };

    public static readonly int N_BOOT_VALS = BOOT_DATA_NAMES.Length;
    public static readonly String SEPARATOR_LINE =
        "--------------------------------------------------";

    /// <summary>
    /// Lists the standard logs on the current computer.
    /// </summary>
    /// <returns>A String with info.</returns>
    public static String listLogs() {
        StringBuilder sb = new StringBuilder();
        EventLog[] remoteEventLogs;
        // Gets logs on the local computer, give remote computer name to get the
        // logs on the remote computer.
        remoteEventLogs = EventLog.GetEventLogs(System.Environment.MachineName);
        sb.AppendLine("Number of logs on computer: " + remoteEventLogs.Length);
        for (int i = 0; i < remoteEventLogs.Length; i++)
            sb.AppendLine("Log: " + remoteEventLogs[i].Log);
        return sb.ToString();
    }

    /// <summary>
    /// Gets provider data for the Microsoft-Windows-Diagnostics-Performance provider.
    /// </summary>
    /// <returns>A String with info.</returns>
    public static String getDiagnosticsEventProvider() {
        return getEventProvider("Microsoft-Windows-Diagnostics-Performance");
    }

    /// <summary>
    /// Gets provider data for the specified provider.
    /// </summary>
    /// <param name="providerName">The name of the provider.</param>
    /// <returns>A String with info.</returns>
    public static String getEventProvider(String providerName) {
        EventLogSession session = new EventLogSession();
        String providerMessageFilePath = String.Empty;
        String providerParameterFilePath = String.Empty;
        String providerResourceFileName = String.Empty;
        String providerHelpLink = String.Empty;
        String providerDisplayName = String.Empty;
        //IList<EventLogLink> eventLogLinks = null;
        StringBuilder sb = new StringBuilder();
        try {
            sb.AppendLine("Provider: " + providerName);
            ProviderMetadata metaData = new ProviderMetadata(providerName,   //Provider name to look up
                                        session,                             //The session
                                        CultureInfo.CurrentCulture);         //Culture of active thread

            if (metaData != null) {
                providerDisplayName = metaData.DisplayName;             //Display Name of the provider
                if (metaData.HelpLink != null)
                    providerHelpLink = metaData.HelpLink.ToString();    //Link to external help on event
                providerMessageFilePath = metaData.MessageFilePath;     //Source of provider metadata
                providerParameterFilePath = metaData.ParameterFilePath;     //Source of provider metadata
                providerResourceFileName = metaData.ResourceFilePath;   //Resource with provider metadata
                //eventLogLinks = metaData.LogLinks;
            }
            //sb.AppendLine("HelpLink: " + providerHelpLink);
            //sb.AppendLine("MessageFilePath: " + providerMessageFilePath);
            //sb.AppendLine("ParameterFilePath: " + providerParameterFilePath);
            //sb.AppendLine("ResourceFilePath: " + providerResourceFileName);

            sb.AppendLine();
            sb.AppendLine("This provider has the following logs:");
            foreach (EventLogLink eventLink in metaData.LogLinks) {
                sb.AppendLine(SEPARATOR_LINE);
                sb.AppendFormat("Display Name: {0}", eventLink.DisplayName);
                sb.AppendFormat("LogName: {0}", eventLink.LogName);
            }

            sb.AppendLine();
            sb.AppendLine("This provider publishes the following events:");
            foreach (EventMetadata eventData in metaData.Events) {
                sb.AppendLine(SEPARATOR_LINE);
                sb.AppendFormat("Event ID = {0}. Description: {1}",
                    eventData.Id, eventData.Description);
            }
        } catch (Exception ex) {
            sb.Append(Utils.excMsg("Error in getEventProviders", ex));
        }
        return sb.ToString();
    }


    /// <summary>
    /// Reads a given log. Doesn't work unless it is one of the standard logs.
    /// </summary>
    /// <param name="logName">Can be Application, Security, System or any other Custom Log.</param>
    /// <param name="nToRead">Number of lines back from the end to read.</param>
    /// <returns>A String with info.</returns>
    public static String readLog(String logName, int nToRead) {
        // The EventLog constructor is passed a String variable for
        // the log name and second argument mention the computer name that you
        // want to read the logs from, and that you have appropriate
        // permissions to

        StringBuilder sb = new StringBuilder();
        try {
            EventLog ev = new EventLog(logName, System.Environment.MachineName);
            sb.AppendLine("Log: " + ev.LogDisplayName);

            int LastLogToShow = ev.Entries.Count;
            if (LastLogToShow <= 0) {
                sb.AppendLine("No Event Logs in the Log: " + logName);
            } else {
                sb.AppendLine("Number of entries in the log: " + ev.Entries.Count);
            }
            // Read the last nToRead records in the specified log.
            if (nToRead > ev.Entries.Count) {
                nToRead = ev.Entries.Count;
            }
            sb.AppendLine("Reading last " + nToRead + " entries");
            int i;
            for (i = ev.Entries.Count - 1; i >= LastLogToShow - nToRead; i--) {
                EventLogEntry CurrentEntry = ev.Entries[i];
                sb.AppendLine(SEPARATOR_LINE);
                sb.AppendLine("Entry: " + (i + 1) + " From End: " + (LastLogToShow - i - 1));
                sb.AppendLine("Event ID:  " + CurrentEntry.InstanceId);
                sb.AppendLine("Entry Type:  " + CurrentEntry.EntryType.ToString());
                sb.AppendLine("Message:   " + CurrentEntry.Message);
            }

            ev.Close();

            /* Similarly you can loop through all the entries in the log using
             * the entries collection as shown in the following commented code.
             * For Each entry In ev.Entries */
        } catch (Exception ex) {
            sb.AppendLine(Utils.excMsg("Error in readDiagnostics", ex));
        }
        sb.AppendLine(SEPARATOR_LINE);
        sb.AppendLine("End reading " + logName);
        return sb.ToString();
    }

    /// <summary>
    /// Gets XML data for a provider.  Works for all logs.
    /// </summary>
    /// <param name="queryString">Usually "*"</param>
    /// <param name="type">PathType.LogName or PathType.FilePath</param>
    /// <param name="logName">
    /// Name of the log.  E.g. "Microsoft-Windows-Diagnostics-Performance/Operational"
    /// </param>
    /// <returns>A String with info.</returns>
    public static String queryLogFileXml(String queryString, PathType type, String logName) {
        StringBuilder sb = new StringBuilder();
        sb.AppendLine("Querying " + logName);
        EventLogQuery eventsQuery = new EventLogQuery(logName, type, queryString);
        int nItems = 0;
        try {
            EventLogReader logReader = new EventLogReader(eventsQuery);
            // Display event info
            for (EventRecord eventInstance = logReader.ReadEvent();
                eventInstance != null; eventInstance = logReader.ReadEvent()) {
                sb.AppendLine(SEPARATOR_LINE);
                String xml = eventInstance.ToXml();
#if FALSE
                    XmlDocument doc = new XmlDocument();
                    doc.LoadXml(xml);
                    String info = XMLUtils.parseDoc(doc);
                    sb.AppendLine(info);
#else
                sb.AppendLine(xml);
#endif
                nItems++;
            }
        } catch (EventLogNotFoundException ex) {
            sb.AppendLine(Utils.excMsg("Could not find the external log to query!", ex));
        } catch (Exception ex) {
            sb.AppendLine(Utils.excMsg("Error in queryLogFileXml", ex));
        }
        sb.AppendLine(SEPARATOR_LINE);
        sb.AppendLine("Number of events: " + nItems);
        sb.AppendLine("End querying " + logName);
        return sb.ToString();
    }

    /// <summary>
    /// Calculates the min, max, mean, and (sample) standard deviation of the columns
    /// in a matrix of values.  The first coluumn is assumed to not be numbers and is
    /// not calculated.  Four items are added to the list, one with the min values,
    /// one with the max values, one with the mean values, and one with the standard
    /// deviations.  The first column has the labels "Min", Max", "Mean", and
    /// "Standard Deviation". The values are written with two decimal spaces (using
    /// format "{0:0.00}").
    /// </summary>
    /// <param name="list">The input list.</param>
    public static void setStatistics(List<String[]> list) {
        if (list == null || list.Count == 0) {
            return;
        }
        int count = list.Count;
        int nItems = list[0].Length;
        if (nItems <= 1) {
            return;
        }
        double[] sum = new double[nItems];
        double[] sum2 = new double[nItems];
        double[] min = new double[nItems];
        double[] max = new double[nItems];
        for (int i = 0; i < nItems; i++) {
            sum[i] = 0;
            sum2[i] = 0;
            min[i] = Double.MaxValue;
            max[i] = Double.MinValue;
        }
        double[] mean = new double[nItems];
        double[] stdev = new double[nItems];
        double doubleVal;
        foreach (String[] array in list) {
            // Accumulate sums for all but the first items in the array
            for (int i = 1; i < nItems; i++) {
                // Convert to double
                // Handle boolean as 0 or 1
                if (array[i].Equals("true")) {
                    doubleVal = 1;
                } else if (array[i].Equals("false")) {
                    doubleVal = 0;
                } else {
                    try {
                        doubleVal = Convert.ToDouble(array[i]);
                    } catch (FormatException) {
                        doubleVal = Double.NaN;
                    } catch (OverflowException) {
                        doubleVal = Double.NaN;
                    }
                }
                sum[i] += doubleVal;
                sum2[i] += doubleVal * doubleVal;
                if (doubleVal < min[i]) {
                    min[i] = doubleVal;
                }
                if (doubleVal > max[i]) {
                    max[i] = doubleVal;
                }
            }
            for (int i = 1; i < nItems; i++) {
                mean[i] = sum[i] / count;
                if (count == 1) {
                    stdev[i] = 0;
                } else {
                    stdev[i] = Math.Sqrt((sum2[i] - count * mean[i] * mean[i]) / (count - 1));
                }
            }
        }
        String[] meanString = new String[nItems];
        meanString[0] = "Mean";
        String[] stdevString = new String[nItems];
        stdevString[0] = "Standard Deviation";
        String[] minString = new String[nItems];
        minString[0] = "Min";
        String[] maxString = new String[nItems];
        maxString[0] = "Max";
        for (int i = 1; i < nItems; i++) {
            meanString[i] = String.Format("{0:0.00}", mean[i]);
            stdevString[i] = String.Format("{0:0.00}", stdev[i]);
            if (min[i] == Double.MaxValue) {
                minString[i] = "NA";
            } else {
                minString[i] = String.Format("{0:0.00}", min[i]);
            }
            if (max[i] == Double.MinValue) {
                maxString[i] = "NA";
            } else {
                maxString[i] = String.Format("{0:0.00}", max[i]);
            }
        }
        list.Add(minString);
        list.Add(maxString);
        list.Add(meanString);
        list.Add(stdevString);
    }


    // 0: BootStartTime
    // 1: BootTime
    // 2: MainPathBootTime
    // 3: BootPostBootTime
    /// <summary>
    /// Gets a List of int[N_BOOT_VALS] values representing the boot times.
    /// <list type="bullet">
    /// <item>0: BootStartTime</item>
    /// <item>1: BootTime</item>
    /// <item>2: MainPathBootTime</item>
    /// <item>3: BootPostBootTime</item>
    /// </list>
    /// </summary>
    /// <returns>The List.</returns>
    public static List<String[]> getBootTimes() {
        //String queryString = "*[System/Level=2]"; // XPATH Query
        String queryString = "*"; // XPATH Query
        String logName = "Microsoft-Windows-Diagnostics-Performance/Operational";
        PathType pathType = PathType.LogName;

        List<String[]> list = new List<String[]>();
        EventLogQuery eventsQuery = new EventLogQuery(logName, pathType, queryString);
        int nItems = 0;
        try {
            EventLogReader logReader = new EventLogReader(eventsQuery);
            for (EventRecord eventInstance = logReader.ReadEvent();
                eventInstance != null; eventInstance = logReader.ReadEvent()) {
                if (eventInstance.Id != 100) {
                    continue;
                }
                String[] vals = new String[N_BOOT_VALS];
                list.Add(vals);
                String xml = eventInstance.ToXml();
                XmlDocument doc = new XmlDocument();
                doc.LoadXml(xml);
                // Assign a prefix to allow accessing the namespace. Select without a namespace
                // gives only items not in a namespace.  All of ours are in the
                // http://schemas.microsoft.com/win/2004/08/events/event namespace.  Thus,
                // we need to specify the namespace.
                XmlNamespaceManager namespaceManager = new XmlNamespaceManager(doc.NameTable);
                namespaceManager.AddNamespace("ms",
                    "http://schemas.microsoft.com/win/2004/08/events/event");
                XPathNavigator nav = doc.CreateNavigator();

                XPathNodeIterator iter;
                int count;
                for (int i = 0; i < N_BOOT_VALS; i++) {
                    iter =
                      nav.Select("/ms:Event/ms:EventData/ms:Data[@Name='" + BOOT_DATA_NAMES[i] + "']",
                      namespaceManager);
                    count = iter.Count;
                    if (count == 0) {
                        vals[i] = "NA";
                    } else {
                        iter.MoveNext();
                        vals[i] = iter.Current.Value;
                    }
                }
                nItems++;
            }
        } catch (EventLogNotFoundException ex) {
            Console.WriteLine(Utils.excMsg("Could not find the " + logName + "log", ex));
            return null;
        } catch (Exception ex) {
            Console.WriteLine(Utils.excMsg("Error in getBootTimes", ex));
            return null;
        }

        return list;
    }

    /// <summary>
    /// Creates a CSV file with boot time information.
    /// </summary>
    /// <param name="fileName">The file name to write to.</param>
    /// <returns>"OK" if successful or an error message.</returns>
    public static String createBootTimesCsvFile(String fileName) {
        TextWriter swBootTimeCsv = null;
        try {
            swBootTimeCsv = new StreamWriter(fileName);

            // Get the boot times
            List<String[]> bootTimes = DiagnosticsUtils.getBootTimes();
            if (bootTimes == null) {
                return "Could not get boot times";
            }

            // Write the headers
            foreach (String name in DiagnosticsUtils.BOOT_DATA_NAMES) {
                swBootTimeCsv.Write(name + ",");
            }
            swBootTimeCsv.WriteLine();

            // Add the statistics
            DiagnosticsUtils.setStatistics(bootTimes);
            // Write the data
            if (bootTimes != null) {
                foreach (String[] array in bootTimes) {
                    foreach (String val in array) {
                        swBootTimeCsv.Write(val + ",");
                    }
                    swBootTimeCsv.WriteLine();
                }
            }
        } catch (Exception ex) {
            return "Failed to write " + fileName + LF + ex.Message;
        } finally {
            // Close the stream
            if (swBootTimeCsv != null) {
                swBootTimeCsv.Close();
            }
        }
        return "OK";
    }

}

