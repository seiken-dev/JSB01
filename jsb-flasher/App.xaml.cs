using System.Configuration;
using System.Data;
using System.Windows;

namespace jsb_flasher;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        string? firmwarePath = null;
        if (e.Args.Length > 0)
        {
            firmwarePath = e.Args[0];
        }

        var mainWindow = new MainWindow(firmwarePath);
        mainWindow.Show();
    }
}

