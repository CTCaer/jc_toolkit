using System;
using System.Windows.Forms;

namespace jcColor
{
    public partial class PresetNameDialog : Form
    {
        public PresetNameDialog()
        {
            InitializeComponent();
            this.AcceptButton = this.m_btn_OK;
        }

        private void btn_OK_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void btn_Cancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

    }
}
