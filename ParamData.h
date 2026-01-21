#ifndef PARAMDATA_H
#define PARAMDATA_H

#include <QString>
#include <QList>

enum ParamType {
    TypeBoolean,    // 纯开关 (如 --embed-subs)
    TypeString,     // 开关 + 文本输入 (如 --proxy "url")
    TypePath,       // 开关 + 文本输入 + 浏览按钮 (如 -P "path")
    TypeAction      // 按钮 (如 -U)
};

struct Category {
    QString id;
    QString title;
    QString iconName;
};

struct Parameter {
    QString id;
    QString categoryId;
    QString flag; // 对于String/Path类型，这是参数名；对于Boolean，这是完整flag
    QString description;
    bool isPopular;
    ParamType type;         // 参数类型
    QString defaultValue;   // 默认值 (用于输入框)
};

class AppData {
public:
    static bool isSettingsCategory(const QString &catId) {
        return catId == "network" || catId == "general";
    }

    static QList<Category> getCategories() {
        return {
            {"format", "视频与格式", "movie"},
            {"audio", "音频提取", "music"},
            {"post", "元数据与字幕", "closed_caption"},
            {"output", "文件与路径", "folder"},
            {"sponsor", "SponsorBlock", "cut"},
            // 下面是设置类
            {"network", "网络与认证", "language"},
            {"general", "通用设置", "settings"}
        };
    }

    static QList<Parameter> getParameters() {
        return {
            // Formats
            {"fmt-best", "format", "-f \"bv+ba/b\"", "下载最佳视频和最佳音频并合并（默认）。", true, TypeBoolean, ""},
            {"fmt-mp4", "format", "-f \"bv*[ext=mp4]+ba[ext=m4a]/b[ext=mp4] / bv*+ba/b\"", "严格首选 MP4/M4A 容器。", true, TypeBoolean, ""},
            {"fmt-merge", "format", "--merge-output-format mp4", "如有必要，合并到 MP4 容器。", true, TypeBoolean, ""},
            {"fmt-res-1080", "format", "-f \"bestvideo[height<=1080]+bestaudio/best[height<=1080]\"", "限制分辨率最高 1080p。", false, TypeBoolean, ""},

            // Audio
            {"aud-extract", "audio", "-x", "仅提取音频轨道。", true, TypeBoolean, ""},
            {"aud-mp3", "audio", "--audio-format mp3", "转换为 MP3 格式。", true, TypeBoolean, ""},
            {"aud-qual", "audio", "--audio-quality 0", "音频质量最佳 (VBR 0)。", false, TypeBoolean, ""},

            // Post
            {"pp-embed-meta", "post", "--embed-metadata", "嵌入元数据。", true, TypeBoolean, ""},
            {"pp-embed-thumb", "post", "--embed-thumbnail", "嵌入缩略图作为封面。", true, TypeBoolean, ""},
            {"pp-subs", "post", "--write-subs", "下载字幕文件。", false, TypeBoolean, ""},
            {"pp-auto-subs", "post", "--write-auto-subs", "下载自动生成的字幕。", true, TypeBoolean, ""},

            // Modified: String Input
            {"pp-sub-langs", "post", "--sub-langs", "指定字幕语言 (正则)。", false, TypeString, "en.*,zh-Hans"},

            {"pp-embed-subs", "post", "--embed-subs", "嵌入字幕到视频容器。", true, TypeBoolean, ""},

            // Output - Modified inputs
            {"fs-template", "output", "-o", "文件名模板。", true, TypeString, "%(title)s.%(ext)s"},
            {"fs-folder", "output", "-P", "指定下载保存路径。", false, TypePath, ""}, // Path selection

            {"fs-restrict", "output", "--restrict-filenames", "限制文件名为 ASCII。", false, TypeBoolean, ""},

            // SponsorBlock
            {"sb-remove", "sponsor", "--sponsorblock-remove all", "移除 SponsorBlock 片段。", true, TypeBoolean, ""},
            {"sb-mark", "sponsor", "--sponsorblock-mark all", "标记 SponsorBlock 片段。", false, TypeBoolean, ""},

            // Network - Modified inputs
            {"net-cookies", "network", "--cookies-from-browser", "从浏览器提取 Cookies (输入浏览器名称，如 chrome, firefox)。", true, TypeString, "chrome"},
            {"net-proxy", "network", "--proxy", "代理服务器地址 (如 http://127.0.0.1:7890)。", false, TypeString, "http://127.0.0.1:7890"},
            {"net-agent", "network", "--user-agent", "伪装 User Agent。", false, TypeString, "Mozilla/5.0"},

            // General
            {"gen-update", "general", "-U", "检查并更新 yt-dlp 程序。", true, TypeAction, ""}, // Action Button
            {"gen-ignore", "general", "-i", "忽略错误并继续。", false, TypeBoolean, ""}
        };
    }
};

#endif // PARAMDATA_H
