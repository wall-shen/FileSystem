make和运行需要的环境：
	zlib
	curllib
	pthread
	
修改 FileLoaderTypes.h 中的 INTERNET_IP 为请求服务器的ip。
修改 FileLoaderTypes.h 中的 UpdateMessageFile 为记录更新的文件。

UpdateMessageFile为一个JSON数组。数组每6个元素分别代表一个更新文件的"目标文件名"，"目标文件位于服务器上的包文件名"，"MD5值"，"所在服务器包文件中的偏移"，"文件大小"，"压缩方式"
例：["1.png", "UpdateData", "MD5", 12, 1024000, 0, "2,png", "UpdateData", "MD52", 1024012, 1024000, 0]
下载文件的任务创建，详见FileManager.cpp
main函数位于testain函数位于test.cpp
编译并运行，程序会比对更新名录，并下载到本地包文件。


各个文件的主要功能：
FileLoader			Loader基类声明
FileLinuxLoader		Linux文件操作
FileWindowsLoader	Windows文件操作
FilePakLoader		包文件操作
FileArichive		序列化
FileInternetLoader	网络请求


服务端：

需要环境：
python3
flask包

运行:
修改 HttpHost.py 里的 DefaultPath 。 DefaultPath为文件所在目录，最后要 '/' 结尾。
运行 python3 HttpHost.py
