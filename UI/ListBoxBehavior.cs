using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;

namespace UI
{
    // ItemsSource가 갱신될 때 마지막 항목으로 자동 스크롤하는 첨부 동작.
    // 코드비하인드 없이(순수 MVVM) XAML에서만 선언해 사용한다.
    public static class ListBoxBehavior
    {
        public static readonly DependencyProperty AutoScrollToEndProperty =
            DependencyProperty.RegisterAttached(
                "AutoScrollToEnd",
                typeof(bool),
                typeof(ListBoxBehavior),
                new PropertyMetadata(false, OnAutoScrollToEndChanged));

        public static bool GetAutoScrollToEnd(DependencyObject obj) => (bool)obj.GetValue(AutoScrollToEndProperty);
        public static void SetAutoScrollToEnd(DependencyObject obj, bool value) => obj.SetValue(AutoScrollToEndProperty, value);

        private static void OnAutoScrollToEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not ListBox listBox || e.NewValue is not true) return;

            if (listBox.Items is INotifyCollectionChanged incc)
            {
                incc.CollectionChanged += (s, args) =>
                {
                    if (args.Action != NotifyCollectionChangedAction.Add) return;

                    // 데이터 추가(동기) 시점과 UI 요소 생성(비동기) 시점의 차이로 발생하는 인덱스 불일치 방지
                    // Dispatcher.BeginInvoke를 사용하여 UI 렌더링이 완료된 후 안전하게 스크롤을 수행
                    listBox.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        if (listBox.Items.Count > 0)
                            listBox.ScrollIntoView(listBox.Items[listBox.Items.Count - 1]);
                    }));
                };
            }
        }
    }
}
