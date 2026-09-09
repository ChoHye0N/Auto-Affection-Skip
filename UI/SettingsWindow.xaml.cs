using System.Globalization;
using System.Windows;

namespace UI
{
    public partial class SettingsWindow : Window
    {
        public AppSettings Settings { get; private set; }

        public SettingsWindow(AppSettings current)
        {
            InitializeComponent();

            Settings = current.Clone();

            ThresholdTextBox.Text = Settings.Threshold.ToString(CultureInfo.InvariantCulture);
            ThresholdStrictTextBox.Text = Settings.ThresholdStrict.ToString(CultureInfo.InvariantCulture);
            DelayTextBox.Text = Settings.DelayMs.ToString(CultureInfo.InvariantCulture);
            RetryCountTextBox.Text = Settings.RetryCount.ToString(CultureInfo.InvariantCulture);
            CaptureOptionComboBox.SelectedIndex = Settings.CaptureOption;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            if (!double.TryParse(ThresholdTextBox.Text, NumberStyles.Float, CultureInfo.InvariantCulture, out var threshold) ||
                threshold < 0.0 || threshold > 1.0)
            {
                MessageBox.Show("임계값(일반)은 0.0 ~ 1.0 사이의 숫자여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!double.TryParse(ThresholdStrictTextBox.Text, NumberStyles.Float, CultureInfo.InvariantCulture, out var thresholdStrict) ||
                thresholdStrict < 0.0 || thresholdStrict > 1.0)
            {
                MessageBox.Show("임계값(엄격)은 0.0 ~ 1.0 사이의 숫자여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!int.TryParse(DelayTextBox.Text, out var delayMs) || delayMs < 0)
            {
                MessageBox.Show("지연 시간은 0 이상의 정수여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!int.TryParse(RetryCountTextBox.Text, out var retryCount) || retryCount < 1)
            {
                MessageBox.Show("응답 대기 재시도 횟수는 1 이상의 정수여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            Settings.Threshold = threshold;
            Settings.ThresholdStrict = thresholdStrict;
            Settings.DelayMs = delayMs;
            Settings.RetryCount = retryCount;
            Settings.CaptureOption = CaptureOptionComboBox.SelectedIndex >= 0 ? CaptureOptionComboBox.SelectedIndex : 0;

            Settings.Save();

            DialogResult = true;
            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }
    }
}
