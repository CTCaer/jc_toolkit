namespace jcColor
{
    partial class PresetNameDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.txtbox_presetName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.m_btn_OK = new System.Windows.Forms.Button();
            this.m_btn_Cancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtbox_presetName
            // 
            this.txtbox_presetName.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.txtbox_presetName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtbox_presetName.Font = new System.Drawing.Font("Segoe UI", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtbox_presetName.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.txtbox_presetName.Location = new System.Drawing.Point(12, 45);
            this.txtbox_presetName.Name = "txtbox_presetName";
            this.txtbox_presetName.Size = new System.Drawing.Size(245, 20);
            this.txtbox_presetName.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.label1.Location = new System.Drawing.Point(9, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(131, 17);
            this.label1.TabIndex = 3;
            this.label1.Text = "Enter preset\'s name:";
            // 
            // m_btn_OK
            // 
            this.m_btn_OK.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_btn_OK.FlatAppearance.BorderSize = 0;
            this.m_btn_OK.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.m_btn_OK.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_btn_OK.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_btn_OK.Location = new System.Drawing.Point(170, 81);
            this.m_btn_OK.Name = "m_btn_OK";
            this.m_btn_OK.Size = new System.Drawing.Size(87, 30);
            this.m_btn_OK.TabIndex = 5;
            this.m_btn_OK.Text = "OK";
            this.m_btn_OK.UseVisualStyleBackColor = false;
            this.m_btn_OK.Click += new System.EventHandler(this.btn_OK_Click);
            // 
            // m_btn_Cancel
            // 
            this.m_btn_Cancel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_btn_Cancel.FlatAppearance.BorderSize = 0;
            this.m_btn_Cancel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.m_btn_Cancel.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_btn_Cancel.ForeColor = System.Drawing.Color.OrangeRed;
            this.m_btn_Cancel.Location = new System.Drawing.Point(68, 81);
            this.m_btn_Cancel.Name = "m_btn_Cancel";
            this.m_btn_Cancel.Size = new System.Drawing.Size(87, 30);
            this.m_btn_Cancel.TabIndex = 6;
            this.m_btn_Cancel.Text = "Cancel";
            this.m_btn_Cancel.UseVisualStyleBackColor = false;
            this.m_btn_Cancel.Click += new System.EventHandler(this.btn_Cancel_Click);
            // 
            // PresetNameDialog
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.ClientSize = new System.Drawing.Size(267, 125);
            this.Controls.Add(this.m_btn_Cancel);
            this.Controls.Add(this.m_btn_OK);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtbox_presetName);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PresetNameDialog";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Update Preset";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        public System.Windows.Forms.TextBox txtbox_presetName;
        private System.Windows.Forms.Label label1;
        public System.Windows.Forms.Button m_btn_OK;
        public System.Windows.Forms.Button m_btn_Cancel;
    }
}