using System;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {

    class Utils {
        public static readonly String LF = "\r\n";

        public static void errMsg(String msg) {
            MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void excMsg(String msg, Exception ex) {
            MessageBox.Show(msg + ":" + LF + ex.Message, "Error",
                MessageBoxButtons.OK, MessageBoxIcon.Error);
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
