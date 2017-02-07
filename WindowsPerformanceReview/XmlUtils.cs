using System;
using System.Text;
using System.Xml;
using System.Xml.XPath;

namespace WindowsPerformanceReview {
    class XmlUtils {
        public static String parseDoc(XmlDocument doc) {
            if (doc == null) {
                return null;
            }
            StringBuilder sb = new StringBuilder();
            XPathNavigator nav = doc.CreateNavigator();
            nav.MoveToRoot();
            sb.AppendLine(nav.Name);
            do {
                //Determine whether children exist.
                if (nav.HasChildren == true) {
                    //Move to the first child.
                    nav.MoveToFirstChild();
                    //Loop through all of the children.
                    do {
                        //Display the data.
                        sb.AppendLine(nav.Name);
                    } while (nav.MoveToNext());
                }
            } while (nav.MoveToNext());
            return sb.ToString();
        }
    }
}
