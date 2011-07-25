#undef FALSE
#define TRUE
#define USE_NAMESPACE

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml.XPath;
using System.Xml;

namespace WindowsPerformanceReview {
    // Class and method to test XPath select expressions.
    class Test {

        public static String test() {
            String xml = "<Event xmlns='http://schemas.microsoft.com/win/2004/08/events/event'><System><Provider Name='Microsoft-Windows-Diagnostics-Performance' Guid='{CFC18EC0-96B1-4EBA-961B-622CAEE05B0A}'/><EventID>100</EventID><Version>2</Version><Level>2</Level><Task>4002</Task><Opcode>34</Opcode><Keywords>0x8000000000010000</Keywords><TimeCreated SystemTime='2010-04-17T15:32:15.799499600Z'/><EventRecordID>3</EventRecordID><Correlation ActivityID='{00000100-0000-0000-E4B2-25C342DECA01}'/><Execution ProcessID='1512' ThreadID='3036'/><Channel>Microsoft-Windows-Diagnostics-Performance/Operational</Channel><Computer>DDCJX4K1</Computer><Security UserID='S-1-5-19'/></System><EventData><Data Name='BootTsVersion'>2</Data><Data Name='BootStartTime'>2010-04-17T15:29:25.671600300Z</Data><Data Name='BootEndTime'>2010-04-17T15:32:12.929094600Z</Data><Data Name='SystemBootInstance'>3</Data><Data Name='UserBootInstance'>2</Data><Data Name='BootTime'>110987</Data><Data Name='MainPathBootTime'>71887</Data><Data Name='BootKernelInitTime'>29</Data><Data Name='BootDriverInitTime'>1930</Data><Data Name='BootDevicesInitTime'>3099</Data><Data Name='BootPrefetchInitTime'>35477</Data><Data Name='BootPrefetchBytes'>314535936</Data><Data Name='BootAutoChkTime'>0</Data><Data Name='BootSmssInitTime'>11453</Data><Data Name='BootCriticalServicesInitTime'>324</Data><Data Name='BootUserProfileProcessingTime'>137</Data><Data Name='BootMachineProfileProcessingTime'>1</Data><Data Name='BootExplorerInitTime'>0</Data><Data Name='BootNumStartupApps'>0</Data><Data Name='BootPostBootTime'>39100</Data><Data Name='BootIsRebootAfterInstall'>false</Data><Data Name='BootRootCauseStepImprovementBits'>0</Data><Data Name='BootRootCauseGradualImprovementBits'>0</Data><Data Name='BootRootCauseStepDegradationBits'>9453889</Data><Data Name='BootRootCauseGradualDegradationBits'>9453889</Data><Data Name='BootIsDegradation'>true</Data><Data Name='BootIsStepDegradation'>true</Data><Data Name='BootIsGradualDegradation'>true</Data><Data Name='BootImprovementDelta'>0</Data><Data Name='BootDegradationDelta'>48988</Data><Data Name='BootIsRootCauseIdentified'>true</Data><Data Name='OSLoaderDuration'>5600</Data><Data Name='BootPNPInitStartTimeMS'>29</Data><Data Name='BootPNPInitDuration'>3206</Data><Data Name='OtherKernelInitDuration'>8428</Data><Data Name='SystemPNPInitStartTimeMS'>11615</Data><Data Name='SystemPNPInitDuration'>1823</Data><Data Name='SessionInitStartTimeMS'>13457</Data><Data Name='Session0InitDuration'>9208</Data><Data Name='Session1InitDuration'>1382</Data><Data Name='SessionInitOtherDuration'>862</Data><Data Name='WinLogonStartTimeMS'>24911</Data><Data Name='OtherLogonInitActivityDuration'>46838</Data><Data Name='UserLogonWaitDuration'>4518</Data></EventData></Event>";
            StringBuilder sb = new StringBuilder();
            sb.AppendLine("Test XPath Navigation");

            XmlDocument doc = new XmlDocument();
            doc.LoadXml(xml);

#if USE_NAMESPACE
            // Assign a short prefix for the namespace.
            XmlNamespaceManager namespaceManager = new XmlNamespaceManager(doc.NameTable);
            namespaceManager.AddNamespace("e", "http://schemas.microsoft.com/win/2004/08/events/event");
#endif

            XmlElement element = doc.DocumentElement;
            XmlNode firstChild = doc.FirstChild;
            XmlNode lastChild = doc.LastChild;
            XmlNodeList childNodes = doc.ChildNodes;
            String name = doc.Name;

            sb.AppendLine("element.Name=" + element.Name);
            sb.AppendLine("firstChild.Name=" + firstChild.Name);
            sb.AppendLine("lastChild.Name=" + lastChild.Name);
            sb.AppendLine("element.Count=" + childNodes.Count);
            sb.AppendLine();

            XPathNavigator nav = doc.CreateNavigator();

            String[] exprs = {
                                 "*",
                                 "/*", 
                                 "//*", 
                                 "/Event",
                                 "//Event",
                                 "/Event/EventData",
                                 "*/Event/EventData",
                                 "*[System/Level=2]",
                                 "//Data",
                                 "/e:Event/e:EventData/e:Data[@Name=BootTime]",
                                 "/e:Event/e:EventData",
                                 "*/e:Event/e:EventData",
                                 "/e:Event/e:EventData/e:Data",
                                 "/e:Event/e:EventData/e:Data[@Name='BootTime']",
                                 "//e:Data[@Name=BootTime]",
                             };
            int count;
            XPathNodeIterator iter;
            foreach (String expr in exprs) {
#if USE_NAMESPACE
                iter = nav.Select(expr, namespaceManager);
#else
                iter = nav.Select(expr);
#endif
                count = iter.Count;
                sb.AppendLine("expr=" + expr + " count=" + count);
            }
            sb.AppendLine("End Test");
            return sb.ToString();
        }

#if FALSE
        /////////////////////////////////////////////////////////////////////
        // Main
        /////////////////////////////////////////////////////////////////////
        static void Main(String[] args) {
            Console.WriteLine(Test.test());
        }
#endif

    }
}
