using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace UI
{
    // 화면 전환만 담당하는 셸(shell) ViewModel. 실제 화면별 상태/로직은 각 화면 ViewModel이 소유한다.
    public class MainViewModel : INotifyPropertyChanged
    {
        public MainScreenViewModel MainScreen { get; }

        private object _currentViewModel;
        public object CurrentViewModel
        {
            get => _currentViewModel;
            private set
            {
                _currentViewModel = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsSettingsNavEnabled));
                OnPropertyChanged(nameof(IsHomeNavEnabled));
            }
        }

        // 현재 화면이 어디인지 아이콘 버튼의 활성/비활성으로 보여주기 위한 값
        public bool IsSettingsNavEnabled => CurrentViewModel == MainScreen && !MainScreen.IsRunning;
        public bool IsHomeNavEnabled => CurrentViewModel != MainScreen;

        public ICommand NavigateToSettingsCommand { get; }
        public ICommand NavigateToMainCommand { get; }

        public MainViewModel()
        {
            MainScreen = new MainScreenViewModel();
            MainScreen.PropertyChanged += MainScreen_PropertyChanged;
            _currentViewModel = MainScreen;

            NavigateToSettingsCommand = new RelayCommand(_ => ShowSettings(), _ => IsSettingsNavEnabled);
            NavigateToMainCommand = new RelayCommand(_ => CurrentViewModel = MainScreen, _ => IsHomeNavEnabled);
        }

        private void MainScreen_PropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(MainScreenViewModel.IsRunning))
            {
                OnPropertyChanged(nameof(IsSettingsNavEnabled));
            }
        }

        private void ShowSettings()
        {
            var settingsViewModel = new SettingsScreenViewModel(MainScreen.GetSettingsSnapshot());
            settingsViewModel.Saved += SettingsViewModel_Saved;
            settingsViewModel.Cancelled += (s, e) => CurrentViewModel = MainScreen;

            CurrentViewModel = settingsViewModel;
        }

        private void SettingsViewModel_Saved(object? sender, AppSettings settings)
        {
            MainScreen.ApplySettings(settings);
            CurrentViewModel = MainScreen;
        }

        // UI 데이터 바인딩 시 값 변경을 알리기 위한 이벤트
        public event PropertyChangedEventHandler? PropertyChanged;

        // 프로퍼티 값이 변경될 때 UI에 갱신을 요청하는 메서드
        protected void OnPropertyChanged([CallerMemberName] string? name = null)
            => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
