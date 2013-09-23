namespace ANPR
{
    partial class Frame
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
            this.snapshotBox = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.snapshotBox)).BeginInit();
            this.SuspendLayout();
            // 
            // snapshotBox
            // 
            this.snapshotBox.Location = new System.Drawing.Point(12, 12);
            this.snapshotBox.Name = "snapshotBox";
            this.snapshotBox.Size = new System.Drawing.Size(555, 378);
            this.snapshotBox.TabIndex = 0;
            this.snapshotBox.TabStop = false;
            // 
            // Frame
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(579, 402);
            this.Controls.Add(this.snapshotBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "Frame";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "ANPR";
            ((System.ComponentModel.ISupportInitialize)(this.snapshotBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox snapshotBox;
    }
}

