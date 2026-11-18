using System.ComponentModel;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;

namespace UI
{
    public class SettingsScreenViewModel : INotifyPropertyChanged
    {
        public event EventHandler<AppSettings>? Saved;
        public event EventHandler? Cancelled;

        private string _thresholdText;
        public string ThresholdText
        {
            get => _thresholdText;
            set { _thresholdText = value; OnPropertyChanged(); }
        }

        private string _thresholdStrictText;
        public string ThresholdStrictText
        {
            get => _thresholdStrictText;
            set { _thresholdStrictText = value; OnPropertyChanged(); }
        }

        private string _delayText;
        public string DelayText
        {
            get => _delayText;
            set { _delayText = value; OnPropertyChanged(); }
        }

        private string _retryCountText;
        public string RetryCountText
        {
            get => _retryCountText;
            set { _retryCountText = value; OnPropertyChanged(); }
        }

        private string _pollIntervalText;
        public string PollIntervalText
        {
            get => _pollIntervalText;
            set { _pollIntervalText = value; OnPropertyChanged(); }
        }

        private int _selectedCaptureOption;
        public int SelectedCaptureOption
        {
            get => _selectedCaptureOption;
            set { _selectedCaptureOption = value; OnPropertyChanged(); }
        }

        public ICommand SaveCommand { get; }
        public ICommand CancelCommand { get; }
        public ICommand ResetDefaultsCommand { get; }

        public SettingsScreenViewModel(AppSettings current)
        {
            _thresholdText = current.Threshold.ToString(CultureInfo.InvariantCulture);
            _thresholdStrictText = current.ThresholdStrict.ToString(CultureInfo.InvariantCulture);
            _delayText = current.DelayMs.ToString(CultureInfo.InvariantCulture);
            _retryCountText = current.RetryCount.ToString(CultureInfo.InvariantCulture);
            _pollIntervalText = current.PollIntervalMs.ToString(CultureInfo.InvariantCulture);
            _selectedCaptureOption = current.CaptureOption;

            SaveCommand = new RelayCommand(_ => Save());
            CancelCommand = new RelayCommand(_ => Cancelled?.Invoke(this, EventArgs.Empty));
            ResetDefaultsCommand = new RelayCommand(_ => ResetDefaults());
        }

        // 입력창만 기본값으로 되돌림 (저장 버튼을 눌러야 반영됨)
        private void ResetDefaults()
        {
            var defaults = new AppSettings();
            ThresholdText = defaults.Threshold.ToString(CultureInfo.InvariantCulture);
            ThresholdStrictText = defaults.ThresholdStrict.ToString(CultureInfo.InvariantCulture);
            DelayText = defaults.DelayMs.ToString(CultureInfo.InvariantCulture);
            RetryCountText = defaults.RetryCount.ToString(CultureInfo.InvariantCulture);
            PollIntervalText = defaults.PollIntervalMs.ToString(CultureInfo.InvariantCulture);
            SelectedCaptureOption = defaults.CaptureOption;
        }

        private void Save()
        {
            if (!double.TryParse(ThresholdText, NumberStyles.Float, CultureInfo.InvariantCulture, out var threshold) ||
                threshold < 0.0 || threshold > 1.0)
            {
                MessageBox.Show("임계값(일반)은 0.0 ~ 1.0 사이의 숫자여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!double.TryParse(ThresholdStrictText, NumberStyles.Float, CultureInfo.InvariantCulture, out var thresholdStrict) ||
                thresholdStrict < 0.0 || thresholdStrict > 1.0)
            {
                MessageBox.Show("임계값(엄격)은 0.0 ~ 1.0 사이의 숫자여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!int.TryParse(DelayText, out var delayMs) || delayMs < 0)
            {
                MessageBox.Show("지연 시간은 0 이상의 정수여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!int.TryParse(RetryCountText, out var retryCount) || retryCount < 1)
            {
                MessageBox.Show("응답 대기 재시도 횟수는 1 이상의 정수여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!int.TryParse(PollIntervalText, out var pollIntervalMs) || pollIntervalMs < 100)
            {
                MessageBox.Show("응답 확인 간격은 100 이상의 정수(ms)여야 합니다.", "입력 오류", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var settings = new AppSettings
            {
                Threshold = threshold,
                ThresholdStrict = thresholdStrict,
                DelayMs = delayMs,
                RetryCount = retryCount,
                PollIntervalMs = pollIntervalMs,
                CaptureOption = SelectedCaptureOption
            };

            settings.Save();
            Saved?.Invoke(this, settings);
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string? name = null)
            => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
