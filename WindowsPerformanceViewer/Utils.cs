using System;
using System.Drawing;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {

    class Utils {
        public static readonly String LF = "\r\n";

        public static void infoMsg(String msg) {
            MessageBox.Show(msg, "Error", MessageBoxButtons.OK,
                MessageBoxIcon.Information);
        }

        public static void errMsg(String msg) {
            MessageBox.Show(msg, "Error", MessageBoxButtons.OK,
                MessageBoxIcon.Error);
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

        /// <summary>
        /// Gets an adjjusted width and height for the given width and height
        /// by multiplying them by the current DPI for the given form divided
        /// by 96, the standard DPI.
        /// </summary>
        /// <param name="form"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        public static Size getDpiAdjustedSize(Form form, Size size) {
            Graphics g = form.CreateGraphics();
            float dpiX = g.DpiX;
            float dpiY = g.DpiY;
            g.Dispose();
            return new Size((int)(dpiX * size.Width / 96F),
                (int)(dpiY * size.Height / 96F));
        }
    }
}
