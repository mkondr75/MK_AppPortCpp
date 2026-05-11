#include "../include/init_config.hpp"
#include "node.hpp"

void FillDefaultConfig(Config &cfg) {
	//"node_mode": "NORMAL",	//TODO enum class NodeMode { OFF, NORMAL };
	cfg.node.mode = NodeMode::NORMAL;
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
	// cfg.node.args = L"--max-old-space-size=192 "
	// 				L"--max-semi-space-size=16 "
	// 				L"--expose-gc "
	// 				L"--permission "
	// 				L"--allow-fs-read=./* "
	// 				L"--allow-fs-write=./* "
	// 				L"--allow-fs-read=C:\\dev_node\\trilium_dev "
	// 				L"--allow-fs-write=C:\\dev_node\\trilium_dev "
	// 				L"--allow-child-process "
	// 				L"--allow-addons";
	cfg.node.args = L"--max-old-space-size=192 --max-semi-space-size=16 "
					L"--expose-gc --permission --allow-fs-read=./* --allow-fs-write=./* "
					L"--allow-fs-read=C:\\dev_node\\trilium_dev "
					L"--allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process "
					L"--allow-addons .\\apps\\server\\dist\\main.cjs";
	cfg.edge.use_system_webview2 = true;
	cfg.edge.user_data_dir = L".\\edge_profile";
	cfg.node.node_env = {// {L"NODE_ENV", L"production"},
						 {L"APPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming"},
						 {L"ComSpec", L"C:\\WINDOWS\\system32\\cmd.exe"},
						 {L"HOST", L"127.0.0.1"},
						 {L"LOCALAPPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Local"},
						 {L"PATH", L"C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin;C:\\WINDOWS\\system32"},
						 {L"SYSTEMROOT", L"C:\\WINDOWS"},
						 {L"TEMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp"},
						 {L"TMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp"},
						 {L"TRILIUM_DATA_DIR", L"C:\\dev_node\\trilium_dev\\data_sandbox"},
						 {L"TRILIUM_PORT", L"9090"},
						 {L"USERPROFILE", L"C:\\dev_node\\trilium_dev\\profile"}};
}