using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Xml;
using System.Xml.XPath;

namespace WindowsPerformanceViewer {
    class BootDataModel {
        public static readonly double MS2SEC = .001;

        /// <summary>
        /// Initial value for the boot data info corresponding to how the data
        /// is stored in the event logs.  Only the name may be accurate.
        /// </summary>
        public static readonly DataSetInfo[] BOOT_DATA_INFOS = {
        new DataSetInfo("BootStartTime", "ms", DataType.STRING),
        new DataSetInfo("BootTime", "ms", DataType.STRING),
        new DataSetInfo("MainPathBootTime", "ms", DataType.STRING),
        new DataSetInfo("BootPostBootTime", "ms", DataType.STRING),

        new DataSetInfo("BootTsVersion", "ms", DataType.STRING),
        new DataSetInfo("BootEndTime", "ms", DataType.STRING),
        new DataSetInfo("SystemBootInstance", "ms", DataType.STRING),
        new DataSetInfo("UserBootInstance", "ms", DataType.STRING),
        new DataSetInfo("BootKernelInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootDriverInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootDevicesInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootPrefetchInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootPrefetchBytes", "ms", DataType.STRING),
        new DataSetInfo("BootAutoChkTime", "ms", DataType.STRING),
        new DataSetInfo("BootSmssInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootCriticalServicesInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootUserProfileProcessingTime", "ms", DataType.STRING),
        new DataSetInfo("BootMachineProfileProcessingTime", "ms", DataType.STRING),
        new DataSetInfo("BootExplorerInitTime", "ms", DataType.STRING),
        new DataSetInfo("BootNumStartupApps", "ms", DataType.STRING),
        new DataSetInfo("BootIsRebootAfterInstall", "ms", DataType.STRING),
        new DataSetInfo("BootRootCauseStepImprovementBits", "ms", DataType.STRING),
        new DataSetInfo("BootRootCauseGradualImprovementBits", "ms", DataType.STRING),
        new DataSetInfo("BootRootCauseStepDegradationBits", "ms", DataType.STRING),
        new DataSetInfo("BootRootCauseGradualDegradationBits", "ms", DataType.STRING),
        new DataSetInfo("BootIsDegradation", "ms", DataType.STRING),
        new DataSetInfo("BootIsStepDegradation", "ms", DataType.STRING),
        new DataSetInfo("BootIsGradualDegradation", "ms", DataType.STRING),
        new DataSetInfo("BootImprovementDelta", "ms", DataType.STRING),
        new DataSetInfo("BootDegradationDelta", "ms", DataType.STRING),
        new DataSetInfo("BootIsRootCauseIdentified", "ms", DataType.STRING),
        new DataSetInfo("OSLoaderDuration", "ms", DataType.STRING),
        new DataSetInfo("BootPNPInitStartTimeMS", "ms", DataType.STRING),
        new DataSetInfo("BootPNPInitDuration", "ms", DataType.STRING),
        new DataSetInfo("OtherKernelInitDuration", "ms", DataType.STRING),
        new DataSetInfo("SystemPNPInitStartTimeMS", "ms", DataType.STRING),
        new DataSetInfo("SystemPNPInitDuration", "ms", DataType.STRING),
        new DataSetInfo("SessionInitStartTimeMS", "ms", DataType.STRING),
        new DataSetInfo("Session0InitDuration", "ms", DataType.STRING),
        new DataSetInfo("Session1InitDuration", "ms", DataType.STRING),
        new DataSetInfo("SessionInitOtherDuration", "ms", DataType.STRING),
        new DataSetInfo("WinLogonStartTimeMS", "ms", DataType.STRING),
        new DataSetInfo("OtherLogonInitActivityDuration", "ms", DataType.STRING),
        new DataSetInfo("UserLogonWaitDuration", "ms", DataType.STRING),
        };

        public static readonly int N_DATA_SETS = BOOT_DATA_INFOS.Length;

        /// <summary>
        /// The boot data info as determined by parsing the logs.
        /// </summary>
        private DataSetInfo[] bootDataInfos =
            (DataSetInfo[])BOOT_DATA_INFOS.Clone();
        internal DataSetInfo[] BootDataInfos {
            get { return bootDataInfos; }
            set { bootDataInfos = value; }
        }
        private double[][] data = null;
        public double[][] Data {
            get { return data; }
            set { data = value; }
        }

        /// <summary>
        /// Gets the data from the event log and puts it in stringData.  This
        /// data is the raw string data from the logs.  Times are in "ms".
        /// Dates are Strings.  Some data is Boolean.
        /// </summary>
        /// <returns>The array or null if the method fails.</returns>
        public String[][] getEventData() {
            //String queryString = "*[System/Level=2]"; // XPATH Query
            String queryString = "*"; // XPATH Query
            String logName =
                "Microsoft-Windows-Diagnostics-Performance/Operational";
            PathType pathType = PathType.LogName;

            List<String[]> list = new List<String[]>();
            if (list == null) {
                Utils.errMsg("Cannot create a List to hold the event data");
                return null;
            }
            EventLogQuery eventsQuery = new EventLogQuery(logName, pathType,
                queryString);
            int nItems = 0;
            try {
                EventLogReader logReader = new EventLogReader(eventsQuery);
                for (EventRecord eventInstance = logReader.ReadEvent();
                    eventInstance != null; eventInstance =
                    logReader.ReadEvent()) {
                    if (eventInstance.Id != 100) {
                        continue;
                    }
                    int nBootVals = bootDataInfos.Length;
                    String[] vals = new String[bootDataInfos.Length];
                    list.Add(vals);
                    String xml = eventInstance.ToXml();
                    XmlDocument doc = new XmlDocument();
                    doc.LoadXml(xml);
                    // Assign a prefix to allow accessing the namespace. Select
                    // without a namespace gives only items not in a namespace.
                    // All of ours are in the
                    // http://schemas.microsoft.com/win/2004/08/events/event
                    // namespace.  Thus, we need to specify the namespace.
                    XmlNamespaceManager namespaceManager =
                        new XmlNamespaceManager(doc.NameTable);
                    namespaceManager.AddNamespace("ms",
                        "http://schemas.microsoft.com/win/2004/08/events/event");
                    XPathNavigator nav = doc.CreateNavigator();

                    XPathNodeIterator iter;
                    int count;
                    for (int i = 0; i < nBootVals; i++) {
                        iter =
                          nav.Select("/ms:Event/ms:EventData/ms:Data[@Name='"
                          + bootDataInfos[i].Name + "']",
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
                Utils.excMsg("Could not find the " + logName + "log", ex);
                return null;
            } catch (Exception ex) {
                Utils.excMsg("Error in getBootTimes", ex);
                return null;
            }
            return list.ToArray();
        }

        /// <summary>
        /// Gets the data from the stringData and converts it to an array of
        /// doubles.  Dates are converted to an OLE automation date.  ms are
        /// converted to sec. Booleans are converted to 0,1.
        /// </summary>
        /// <param name="keepStringData">
        /// Whether to null the stringData or not.</param>
        /// <returns>Whether successful or not.</returns>
        public Boolean getData() {
            // First get the event data
            String[][] stringData = getEventData();
            if (stringData == null) {
                return false;
            }
            // The number of data sets
            int count = stringData.Length;
            if (count <= 0) {
                Utils.errMsg("The String data is an empty array");
                return false;
            }
            // The number of items in a data set
            int nItems = stringData[0].Length;
            if (nItems != bootDataInfos.Length) {
                Utils.errMsg("The String data has " + nItems
                + " items, expected " + bootDataInfos.Length + "items");
                return false;
            }
            // Fill the double array, converting the data
            double doubleVal;
            String name;
            Boolean scaleToSec;
            String firstValue;
            data = new double[count][];
            for (int j = 0; j < count; j++) {
                data[j] = new double[nItems];
                scaleToSec = false;
                for (int i = 0; i < nItems; i++) {
                    if (j == 0) {
                        // Set the bootInfosData parameters based on the names
                        // or first values
                        name = bootDataInfos[i].Name;
                        firstValue = stringData[0][i];
                        if (firstValue.Equals("true") ||
                                firstValue.Equals("false")) {
                            // Set names with first value that is true or false
                            // to be Boolean
                            bootDataInfos[i].Units = "Boolean";
                            bootDataInfos[i].Type = DataType.BOOLEAN;

                        } else if (firstValue.EndsWith("Z")) {
                            // Set names with first value that ends with Z to
                            // be dates
                            bootDataInfos[i].Units = "";
                            bootDataInfos[i].Type = DataType.DATE;
                        } else if (name.Contains("Time") || name.Contains("Duration")) {
                            // Scale names of those that are left that contain
                            // Duration or Time to sec
                            scaleToSec = true;
                            bootDataInfos[i].Units = "sec";
                            bootDataInfos[i].Type = DataType.DOUBLE;
                        } else {
                            // Others are numbers
                            bootDataInfos[i].Units = "";
                            bootDataInfos[i].Type = DataType.DOUBLE;
                        }
                    }
                    if (stringData[j][i].Equals("true")) {
                        // Boolean true is 1
                        doubleVal = 1;
                    } else if (stringData[j][i].Equals("false")) {
                        // Boolean false is 0
                        doubleVal = 0;
                    } else if (stringData[j][i].EndsWith("Z")) {
                        // UTC string converts to OLE automation time
                        DateTime dateTime =
                            Utils.utcToLocalDateTime(stringData[j][i]);
                        doubleVal = dateTime.ToOADate();
                    } else {
                        try {
                            doubleVal = Convert.ToDouble(stringData[j][i]);
                        } catch (FormatException) {
                            doubleVal = Double.NaN;
                        } catch (OverflowException) {
                            doubleVal = Double.NaN;
                        }
                        if (scaleToSec) {
                            doubleVal *= MS2SEC;
                        }
                    }
                    data[j][i] = doubleVal;
                }
            }
            return true;
        }

        /// <summary>
        /// Gets the index for a specified data name.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <returns>The index for the name or -1 if not found.</returns>
        public int getIndexFor(String name) {
            for (int i = 0; i < N_DATA_SETS; i++) {
                if (BOOT_DATA_INFOS[i].Name.Equals(name)) {
                    return i;
                }
            }
            return -1;
        }

    }
}
