#include "../include/init_config.hpp"

void FillDefaultConfig(Config &cfg)
{
    cfg.paths.root_dir = L"c:\\msys64\\home\\maxko\\dev\\AppPortCpp";
    cfg.paths.bin_dir = cfg.paths.root_dir + L"\\bin";
    cfg.paths.logs_dir = cfg.paths.bin_dir + L"\\logs";
    cfg.paths.config_file = cfg.paths.bin_dir + L"\\app.ini";
    cfg.paths.node = L"\0"; //"node.exe";
    // cfg.paths.webview2_loader = L".\\WebView2Loader.dll";
    cfg.logs.launcher = LogMode::FILE_MODE;
    cfg.logs.node = LogMode::FILE_MODE;
    cfg.logs.webview = LogMode::FILE_MODE;
    // cfg.logs.launcher_file = L"logs\\launcher.log";
    // cfg.logs.node_file = L"logs\\node.log";
    // cfg.logs.webview_file = L"logs\\webview.log";
    // cfg.logs.crash_file = L"logs\\crash.log";
    cfg.node.exe = L"node.exe";
    cfg.node.working_dir = L"C:\\dev_node\\trilium_dev";
    cfg.node.entry_script = L".\\apps\\server\\dist\\main.cjs";
    cfg.node.args =
        L"--max-old-space-size=192 "
        L"--max-semi-space-size=16 "
        L"--expose-gc "
        L"--permission "
        L"--allow-fs-read=./* "
        L"--allow-fs-write=./* "
        L"--allow-fs-read=C:\\dev_node\\trilium_dev "
        L"--allow-fs-write=C:\\dev_node\\trilium_dev "
        L"--allow-child-process "
        L"--allow-addons";
    cfg.edge.use_system_webview2 = true;
    cfg.edge.user_data_dir = L".\\edge_profile";
}