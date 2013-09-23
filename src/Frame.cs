using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using OpenCvSharp;

namespace ANPR
{
    public partial class Frame : Form
    {
        public Frame()
        {
            InitializeComponent();
            ConfigureEnvironment();
        }

        private void ConfigureEnvironment()
        {
            new Thread(ProcessSnapshot).Start();
        }

        private void ProcessSnapshot()
        {
            try
            {
                CvCapture capture = Cv.CreateCameraCapture(CaptureDevice.Any);
                while (true)
                {
                    IplImage snapshot = capture.QueryFrame();
                    snapshotBox.Image = snapshot.ToBitmap();
                    Application.DoEvents();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}