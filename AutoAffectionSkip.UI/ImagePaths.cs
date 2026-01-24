using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace AutoAffectionSkip.UI
{
    public static class ImagePaths
    {
        public static Dictionary<string, string> GetImageDictionary()
        {
            string baseDir = AppContext.BaseDirectory;
            string[] imageFiles = {
                "bond_event_logo.png", "bond_story_logo.png", "exit_momotalk.png",
                "menu_btn.png", "menu_info_btn.png", "message_count_box.png",
                "message_icon.png", "message_reply_logo.png", "message_story_enter_btn.png",
                "momotalk_icon.png", "momotalk_logo.png", "ok_btn.png",
                "pyroxene.jpg", "skip_btn.png", "story_enter_btn.png", "summary_logo.jpg"
            };

            return imageFiles.ToDictionary(
                name => Path.GetFileNameWithoutExtension(name),
                name => Path.Combine(baseDir, "assets", "images", name)
            );
        }
    }
}