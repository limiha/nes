using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.UI;
using Microsoft.Graphics.Canvas.UI.Xaml;
using NesRuntimeComponent;
using System;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Storage.Pickers;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace nesUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            var args = e.Parameter as Windows.ApplicationModel.Activation.IActivatedEventArgs;
            if (args != null)
            {
                if (args.Kind == Windows.ApplicationModel.Activation.ActivationKind.File)
                {
                    var fileArgs = args as Windows.ApplicationModel.Activation.IFileActivatedEventArgs;
                    _romFile = (Windows.Storage.StorageFile)fileArgs.Files[0];
                }
            }
        }

        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
        }

        private void canvas_Draw(ICanvasAnimatedControl sender, CanvasAnimatedDrawEventArgs args)
        {
            var bitmap = CanvasBitmap.CreateFromBytes(
                sender.Device,
                _bitmapBytes,
                256,
                240,
                Windows.Graphics.DirectX.DirectXPixelFormat.R8G8B8A8UIntNormalized,
                sender.Dpi,
                CanvasAlphaMode.Ignore
                );

            args.DrawingSession.DrawImage(
                bitmap,
                new Rect(0, 0, sender.Size.Width, sender.Size.Height),
                bitmap.Bounds,
                1,
                CanvasImageInterpolation.NearestNeighbor
                );
        }

        private void canvas_Update(ICanvasAnimatedControl sender, CanvasAnimatedUpdateEventArgs args)
        {
            _nes.DoFrame(_bitmapBytes);
        }

        private void canvas_CreateResources(CanvasAnimatedControl sender, CanvasCreateResourcesEventArgs args)
        {
            args.TrackAsyncAction(CreateResourcesAsync(sender).AsAsyncAction());
        }

        private async Task CreateResourcesAsync(CanvasAnimatedControl sender)
        {
            _bitmapBytes = new byte[256 * 240 * 4];

            var file = _romFile;
            if (file == null)
            {
                var picker = new FileOpenPicker();
                picker.FileTypeFilter.Add(".nes");
                file = await picker.PickSingleFileAsync();
            }
            _nes = await Nes.Create(file);
            _controller0 = _nes.GetStandardController(0);
            Window.Current.CoreWindow.KeyDown += HandleKey;
            Window.Current.CoreWindow.KeyUp += HandleKey;
            sender.Width = 256 * 3;
            sender.Height = 240 * 3;
        }

        private void HandleKey(CoreWindow sender, KeyEventArgs args)
        {
            if (args.KeyStatus.RepeatCount == 1)
            {
                bool state = !args.KeyStatus.IsKeyReleased;
                switch (args.VirtualKey)
                {
                    case VirtualKey.S:
                        _controller0.A(state);
                        break;
                    case VirtualKey.A:
                        _controller0.B(state);
                        break;
                    case VirtualKey.Shift:
                        _controller0.Select(state);
                        break;
                    case VirtualKey.Enter:
                        _controller0.Start(state);
                        break;
                    case VirtualKey.Up:
                        _controller0.Up(state);
                        break;
                    case VirtualKey.Down:
                        _controller0.Down(state);
                        break;
                    case VirtualKey.Left:
                        _controller0.Left(state);
                        break;
                    case VirtualKey.Right:
                        _controller0.Right(state);
                        break;
                }
            }
        }

        private Nes _nes;
        private StandardController _controller0;
        private byte[] _bitmapBytes;
        private Windows.Storage.StorageFile _romFile;
    }
}
