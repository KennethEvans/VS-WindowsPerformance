using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace WindowsPerformanceReview {

    class Utils {
        public static readonly String LF = "\r\n";

        public static void excMsg(StreamWriter sw, String msg, Exception ex) {
            sw.WriteLine(msg + ":" + LF + ex.Message);
        }

        public static String excMsg(String msg, Exception ex) {
            return msg + ":" + LF + ex.Message;
        }

        public static String errMsg(String msg) {
            return msg;
        }

        /// <summary>
        /// Converts a string representing UTC time to a DateTime.
        /// </summary>
        /// <param name="utc">The string to convert.</param>
        /// <returns></returns>
        public static DateTime utcToLocalDateTime(String utc) {
            DateTime convertedDate = DateTime.SpecifyKind(
                DateTime.Parse(utc),
                DateTimeKind.Utc);
            DateTime dt = convertedDate.ToLocalTime();
            return dt; 
        }
    }
}
