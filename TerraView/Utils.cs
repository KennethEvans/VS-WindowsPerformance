using System;
using System.Windows.Forms;

namespace KEUtilities
{
	/// <summary>
	/// Error message box.
	/// </summary>
	abstract public class ErrMsg
	{
		private ErrMsg()
		{
		}

        static public void Show(String s) {
            MessageBox.Show(s,"Error");
        }
    }

    /// <summary>
    /// Info message box.
    /// </summary>
    abstract public class InfoMsg {
        private InfoMsg() {
        }

        static public void Show(String s) {
            MessageBox.Show(s,"Information");
        }
    }

    /// <summary>
    /// Warning message box.
    /// </summary>
    abstract public class WarnMsg {
        private WarnMsg() {
        }

        static public void Show(String s) {
            MessageBox.Show(s,"Warning");
        }
    }

    /// <summary>
    /// Exception message box.
    /// </summary>
    abstract public class ExcMsg {
        private ExcMsg() {
        }

        static public void Show(String s) {
            MessageBox.Show(s,"Exception");
        }

        static public void Show(Exception e) {
            MessageBox.Show(e.Message,"Exception");
        }

        static public void Show(String s, Exception e) {
            MessageBox.Show((s == null?"":(s + "\n\n")) + e.ToString(),"Exception");
        }

        static public void ShowDlg(String s, Exception e) {
            ExDlg dlg=new ExDlg(s,e);
            dlg.ShowDialog();
        }
    }
}
