using System.Windows;

namespace UI
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            // 데이터 연결
            var viewModel = new MainViewModel();
            this.DataContext = viewModel;

            // 최신 Log 항목으로 이동
            ((System.Collections.Specialized.INotifyCollectionChanged)viewModel.Logs).CollectionChanged += (s, e) =>
            {
                if (e.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Add)
                {
                    // 데이터 추가(동기) 시점과 UI 요소 생성(비동기) 시점의 차이로 발생하는 인덱스 불일치 방지
                    // Dispatcher.BeginInvoke를 사용하여 UI 렌더링이 완료된 후 안전하게 스크롤을 수행
                    Application.Current.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        if (LogListBox.Items.Count > 0)
                        {
                            // Items.Count가 실제 UI 요소 개수와 일치하는 상태에서 마지막 항목으로 이동
                            var lastItem = LogListBox.Items[LogListBox.Items.Count - 1];
                            LogListBox.ScrollIntoView(lastItem);
                        }
                    }));
                }
            };
        }
    }
}