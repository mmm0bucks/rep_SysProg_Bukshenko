using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace BukshenkoSys1
{
    public partial class Form1 : Form
    {
        Process childProcess = null;
        EventWaitHandle eventStart = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
        EventWaitHandle eventConfirm = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
        EventWaitHandle eventClose = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseProc");
        EventWaitHandle eventExit = new EventWaitHandle(false, EventResetMode.AutoReset, "ExitProc");

        int threadsCounter = 0;
        public Form1()
        {

            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (childProcess == null || childProcess.HasExited)
            {
                comboBox1.Items.Clear();
                childProcess = Process.Start("L01.exe");
                eventConfirm.WaitOne();
                updateList("main");
                updateList("all threads");
                threadsCounter++;

            }
            else
            {
                int n = 0;
                try
                {
                    n = Int32.Parse(textBox1.Text);
                }
                catch
                { }

                for (int i = 0; i < n; ++i)
                {
                    eventStart.Set();
                    eventConfirm.WaitOne();
                    updateList((threadsCounter++).ToString());
                }
            }

        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (childProcess == null || childProcess.HasExited)
            {
                comboBox1.Items.Clear();
                threadsCounter = 0;
            }
            else
            {
                eventClose.Set();
                eventConfirm.WaitOne();

                if (comboBox1.Items.Count > 2)
                    comboBox1.Items.RemoveAt(threadsCounter--);

                else
                {
                    comboBox1.Items.Clear();
                    threadsCounter = 0;
                }
            }
            threadsCounter = 0;

        }

        private void updateList(string name)
        {
            comboBox1.Items.Add(name);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!(childProcess == null || childProcess.HasExited))
            {
                if (threadsCounter == 0)
                {
//                    listBox.Items.Clear();
                    childProcess.Kill();
                }
                else
                {
                    eventExit.Set();
                    if (eventConfirm.WaitOne())
                    {
  //                      listBox.Items.RemoveAt(listBox.Items.Count - 1);
                        threadsCounter--;
                    }
                }
            }
            else
            {
    //            listBox.Items.Clear();
                threadsCounter = 0;
            }
        }

    }
}
