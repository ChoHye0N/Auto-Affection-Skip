using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;

namespace UI
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private const double THRESHOLD = 0.95;

        // 함수 relay 횟수 확인 변수
        private int _countSecond = 0;

        // 메시지 관련 변수
        private ButtonInfo[] _messageList = new ButtonInfo[5];
        private int _messageCount = 0;
        private bool _messageCycle = false;

        private readonly Dictionary<string, string> _images;
        private MacroStep _currentStep = MacroStep.Idle;
        private CancellationTokenSource? _cts;

        private bool _isRunning;
        public bool IsRunning
        {
            get => _isRunning;
            set {
                _isRunning = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsStartEnabled));
            }
        }

        public bool IsStartEnabled => !IsRunning;
        public ObservableCollection<string> Logs { get; } = new ObservableCollection<string>();

        public ICommand StartCommand { get; }
        public ICommand StopCommand { get; }

        public MainViewModel()
        {
            _images = ImagePaths.GetImageDictionary();
            StartCommand = new RelayCommand(async _ => await StartMacroAsync(), _ => !IsRunning);
            StopCommand = new RelayCommand(_ => StopMacro(), _ => IsRunning);
        }

        private void WriteLog(string message)
        {
            if (Application.Current == null) return;

            Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                try
                {
                    Logs.Add($"[{DateTime.Now:HH:mm:ss}] {message}");
                    if (Logs.Count > 100) Logs.RemoveAt(0);
                }
                catch (Exception) { /* 로그 기록 중 발생하는 사소한 오류 무시 */ }
            }));
        }

        public async Task StartMacroAsync()
        {
            if (IsRunning) return;

            IsRunning = true;
            _cts = new CancellationTokenSource();
            _currentStep = MacroStep.CheckMain;

            WriteLog("▶ 매크로 시작");

            try
            {
                // Task.Run 내부를 비동기(async)로 변경하여 Task.Delay 사용 가능하게 함
                await Task.Run(async () => await RunMacroLoopAsync(_cts.Token), _cts.Token);
            }
            catch (OperationCanceledException)
            {
                WriteLog("■ 매크로 중지");
            }
            catch (Exception ex)
            {
                WriteLog($"실행 중 오류: {ex.Message}");
            }
            finally
            {
                Cleanup();
            }
        }

        private void StopMacro()
        {
            if (_cts != null)
            {
                WriteLog("중지 요청 중...");
                _cts.Cancel();
            }
            _currentStep = MacroStep.Finish;
        }

        private void Cleanup()
        {
            // 로그를 먼저 찍고 상태를 변경해야 오류가 발생안함
            WriteLog(">>> 매크로 상태 초기화 완료");

            _currentStep = MacroStep.Idle;
            _cts?.Dispose();
            _cts = null;

            // IsRunning은 가장 마지막에 변경
            Application.Current.Dispatcher.Invoke(() => IsRunning = false);
        }

        // void 대신 async Task를 사용하여 내부에서 await Task.Delay 사용
        private async Task RunMacroLoopAsync(CancellationToken token)
        {
            while (_currentStep != MacroStep.Finish)
            {
                // 즉시 취소 확인
                token.ThrowIfCancellationRequested();

                switch (_currentStep)
                {
                    case MacroStep.CheckMain: CheckMainStep(); break;
                    case MacroStep.EnterMomotalk: await EnterMomotalkStep(token); break;
                    case MacroStep.ScanMessages: await ScanMessagesStep(token); break;
                    case MacroStep.ProcessMessage: await ProcessConversation(token); break;
                    case MacroStep.SkipStory: await HandleStorySkip(token); break;
                    case MacroStep.ClaimReward: await HandleClaimReward(token); break;
                }

                // 루프 사이의 짧은 대기 (취소 토큰 포함)
                await Task.Delay(500, token);
            }
        }

        #region Step Methods
        private void CheckMainStep()
        {
            WriteLog("[1] 메인화면 확인...");

            var res = NativeMethods.FindImage(_images["momotalk_icon"], THRESHOLD);

            if (res.found)
            {
                NativeMethods.MouseClick(res.x, res.y);
                _currentStep = MacroStep.EnterMomotalk;
            }
            else
            {
                WriteLog("안 읽은 메시지 없음!");
                _currentStep = MacroStep.Finish;
            }
        }

        private async Task EnterMomotalkStep(CancellationToken token)
        {
            await Task.Delay(1500, token);
            var res = NativeMethods.FindImage(_images["message_icon"], THRESHOLD);

            if (res.found)
            {
                NativeMethods.MouseClick(res.x, res.y);
                _currentStep = MacroStep.ScanMessages;
            }
        }

        private async Task ScanMessagesStep(CancellationToken token)
        {
            await Task.Delay(1000, token);

            // 처음 인식한 메시지 개수만큼 돌았을 때 확인
            if (_messageCycle)
            {
                WriteLog("읽을 메시지 없음, 메인화면으로 복귀중...");
                NativeMethods.KeyPressScan(0x01);

                _messageCycle = false;
                _currentStep = MacroStep.CheckMain;

                return;
            }

            // 메시지 목록이 비어있을 때만 새로 스캔
            if (_messageCount == 0)
            {
                // 배열 초기화
                Array.Clear(_messageList, 0, _messageList.Length);

                _messageCount = NativeMethods.FindMultiImage(_images["message_count_box"], 0.99, _messageList, _messageList.Length);

                if (_messageCount == 0)
                {
                    WriteLog("읽을 메시지 없음, 매크로 종료!");
                    NativeMethods.KeyPressScan(0x01);

                    StopMacro();
                }

                if (_messageCount > 0)
                {
                    // 화면 위쪽부터 순서대로
                    WriteLog($"{_messageCount}개의 안 읽은 메시지를 발견!");
                    Array.Sort(_messageList, 0, _messageCount, Comparer<ButtonInfo>.Create((a, b) => a.y.CompareTo(b.y)));
                }
            }

            if (_messageCount > 0)
            {
                // 위에서부터 순서대로 클릭하기 위해 0번 인덱스 사용
                var res = _messageList[0];

                NativeMethods.MouseClick(res.x, res.y);
                _currentStep = MacroStep.ProcessMessage;

                // 사용한 항목 제거 (앞으로 당기기)
                for (int i = 0; i < _messageCount - 1; i++)
                {
                    _messageList[i] = _messageList[i + 1];
                }
                _messageCount--;

                if (_messageCount == 0) _messageCycle = true;

                await Task.Delay(500, token);
            }
        }

        private async Task ProcessConversation(CancellationToken token)
        {
            WriteLog("[2] 메시지 읽기 및 답장 중...");

            while (!token.IsCancellationRequested)
            {
                if (NativeMethods.FindImage(_images["message_reply_logo"], 0.99).found)
                {
                    NativeMethods.KeyPressScan(0x02);
                    _countSecond = 0;
                    await Task.Delay(800, token);
                }

                if (NativeMethods.FindImage(_images["message_story_enter_btn"], 0.99).found)
                {
                    NativeMethods.KeyPressScan(0x39);
                    _countSecond = 0;
                    await Task.Delay(800, token);
                    break;
                }
                if (_countSecond >= 5)
                {
                    _currentStep = MacroStep.ScanMessages;
                    _countSecond = 0;
                    break;
                }

                _countSecond++;
                await Task.Delay(800, token);
            }

            if (_currentStep == MacroStep.ScanMessages)
            {
                WriteLog("＃ 새로운 메시지 선택");
                return;
            }

            await WaitForImageAndPushAsync("story_enter_btn", 0x39, token);
            await Task.Delay(5000, token);

            _currentStep = MacroStep.SkipStory;
        }

        private async Task HandleStorySkip(CancellationToken token)
        {
            WriteLog("[3] 스토리 스킵 시작");

            await WaitForImageAndPushAsync("menu_btn", 0x01, token);
            await Task.Delay(800, token);

            //await WaitForImageAndClickAsync("skip_btn", token);
            //await Task.Delay(800, token);

            await WaitForImageAndPushAsync("ok_btn", 0x39, token);
            if (_countSecond >= 5) NativeMethods.KeyPressScan(0x01);
            await Task.Delay(800, token);

            _currentStep = MacroStep.ClaimReward;
        }

        private async Task HandleClaimReward(CancellationToken token)
        {
            WriteLog("[4] 보상 확인");

            await WaitForImageAndPushAsync("pyroxene", 0x1C, token);

            _currentStep = MacroStep.ScanMessages;
        }

        private async Task WaitForImageAndPushAsync(string imgName, ushort key, CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                WriteLog($"[{imgName}] 이미지 찾는 중...");

                var res = NativeMethods.FindImage(_images[imgName], THRESHOLD);

                if (res.found) {
                    NativeMethods.KeyPressScan(key);
                    _countSecond = 0;
                    return;
                }
                _countSecond++;

                await Task.Delay(1000, token);
            }
        }
        #endregion

        // UI 데이터 바인딩 시 값 변경을 알리기 위한 이벤트
        public event PropertyChangedEventHandler? PropertyChanged;

        // 프로퍼티 값이 변경될 때 UI에 갱신을 요청하는 메서드
        protected void OnPropertyChanged([CallerMemberName] string? name = null)
            => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

    // UI의 Command 바인딩을 처리하기 위한 클래스
    public class RelayCommand : ICommand
    {
        private readonly Action<object?> _execute;
        private readonly Func<object?, bool>? _canExecute;
        public RelayCommand(Action<object?> execute, Func<object?, bool>? canExecute = null)
        {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }
        public bool CanExecute(object? parameter) => _canExecute == null || _canExecute(parameter);
        public void Execute(object? parameter) => _execute(parameter);
        public event EventHandler? CanExecuteChanged { add => CommandManager.RequerySuggested += value; remove => CommandManager.RequerySuggested -= value; }
    }
}