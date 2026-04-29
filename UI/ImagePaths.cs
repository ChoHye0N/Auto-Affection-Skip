using System.IO;

namespace UI
{
    public static class ImagePaths
    {
        public static Dictionary<string, string> GetImageDictionary()
        {
            string baseDir = AppContext.BaseDirectory;
            string[] imageFiles = {
                "exit_momotalk.png", "menu_btn.png", "message_count_box.png",
                "message_icon.png", "message_reply_logo_0.png",
                "message_story_enter_btn.png", "momotalk_icon_0.png",
                "momotalk_logo.png", "ok_btn.png", "pyroxene.png", "skip_btn.png",
                "story_enter_btn.png"
            };

            return imageFiles.ToDictionary(
                name => Path.GetFileNameWithoutExtension(name),
                name => Path.Combine(baseDir, "resource", "images", name)
            );
        }
    }
}