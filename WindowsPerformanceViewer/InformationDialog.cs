using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class InformationDialog : Form {
        public System.Windows.Forms.TextBox TextBox {
            get { return textBox; }
            set { textBox = value; }
        }

        public InformationDialog() {
            InitializeComponent();
        }
    }
}
