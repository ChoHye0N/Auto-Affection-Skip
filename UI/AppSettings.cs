using System.IO;
using System.Text.Json;

namespace UI
{
    // 사용자가 설정 창에서 조절 가능한 매크로 옵션
    public class AppSettings
    {
        public double Threshold { get; set; } = 0.95;
        public double ThresholdStrict { get; set; } = 0.99;
        public int DelayMs { get; set; } = 1000;
        public int RetryCount { get; set; } = 5;
        public int CaptureOption { get; set; } = 0; // 0: BitBlt, 1: PrintWindow

        private static readonly string SettingsDir =
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "AutoAffectionSkip");
        private static readonly string SettingsPath = Path.Combine(SettingsDir, "settings.json");

        public static AppSettings Load()
        {
            try
            {
                if (File.Exists(SettingsPath))
                {
                    var json = File.ReadAllText(SettingsPath);
                    var loaded = JsonSerializer.Deserialize<AppSettings>(json);
                    if (loaded != null) return loaded;
                }
            }
            catch (Exception) { /* 파일이 손상된 경우 기본값 사용 */ }

            return new AppSettings();
        }

        public void Save()
        {
            Directory.CreateDirectory(SettingsDir);
            var json = JsonSerializer.Serialize(this, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(SettingsPath, json);
        }

        public AppSettings Clone() => (AppSettings)MemberwiseClone();
    }
}
