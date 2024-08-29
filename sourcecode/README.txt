2024 集成电路创新创业大赛 飞腾杯
作品名称：“智慧之眼：视障人士的户外安全出行伴侣”
团队编号：CICC1085 
团队名称：飞腾风驰队 
日期：2024.8.15

目录说明：
/masterdevice：用于系统Master端的应用程序源代码。
	注意：当前可执行的最新版本在于该目录下的子目录"PySide6-GUI0815"。
	系统运行：需要根据requirements.txt安装Python 3.8及对应的库版本。
	
	进入root用户，运行以下命令运行Master系统：
	sudo -s
	cd master
	python3 main.py
	
	需要注意，该系统需要正确启动Slave端的openamp_core0.elf固件，才能正常运行。
	
/slavedevice：用于系统Slave端的固件源代码，及固件原件。
	固件原件为phytiumpi_aarch64_firefly_phytiumflyers.elf，
	该固件需要配合 Linux Phytium-5.10+ 版本内核、飞腾的Ubuntu Linux
	20.04 LTS 操作系统下使用，置于 /lib/firmware 目录中，覆盖
	openamp_core0.elf文件。
	
	启动该固件的命令行需要进入root用户：
	sudo -s
	echo start > /sys/class/remoteproc/remoteproc0/state
	for dir in /sys/bus/rpmsg/devices/*/; do
		if [ -f "${dir}driver_override" ]; then
			echo "rpmsg_lawrence" > "${dir}driver_override"
		fi
	done
	modprobe rpmsg_lawrence
	
	接下来检查/dev目录是否有生成各个rpmsg endpoint的rpmsg_ctrl*节点，
	若没有生成，则需要将上述echo rpmsg_lawrence过程手动进行，并在必要
	时使用命令 dmesg 检查Slave端是否有正常进行。
	
/kernel_managed：支持软件正常运行的Linux Kernel版本和各项系统驱动目录，以及dtb。
	如作品文档所述，所修改的系统驱动为rpmsg_lawrence。
	Linux Kernel 版本是自行编译的定制版本，运行时请使用此固件版本，以避免驱动程
	序的Version Magic问题。
	

更新记录（自2024.5.20区域赛初赛起至2024.8.15）：
/masterdevice：
	### 截至全国总决赛
	20240815 git_tag:64a7322 git_commit:国赛提交版
	### 截至区域赛决赛
	20240725 git_tag:507aa4d git_commit:删除文件 PySide6-GUI0713/README.md
	20240725 git_tag:2cc050f git_commit:修改了代码结构，定时发送语音播报，跳过语音包解包
	20240613 git_tag:2c35d78 git_commit:转换为mnn格式的模型，未经量化
	20240613 git_tag:25f698e git_commit:使用mnn推理yolov8
	20240518 git_tag:d49e8c4 git_commit:初赛代码（原始版本）
	### 截至区域赛初赛
	
/slavedevice：
	### 截至全国总决赛
	20240812 git_tag:ff668db git_commit:调整了加速度计发送数据包于GPS API解析
	20240812 git_tag:57468ba git_commit:增加GPS解析播报
	### 截至区域赛决赛
	20240725 git_tag:1770853 git_commit:speech发0、1
	20240725 git_tag:e80208a git_commit:增加摔倒判断，修复GPS卡死
	20240724 git_tag:41bdeeb git_commit:修复播报距离不正确
	20240721 git_tag:fbca437 git_commit:运行正常，红绿灯一直是绿灯，数据缓存问题，视角异常未加；增加互斥量
	20240719 git_tag:55a5c9b git_commit:语音解析数据调试
	20240711 git_tag:22d978f git_commit:所有传感器都用任务进行，GPS有小bug，但不影响整体运行
	20240617 git_tag:a97adb4 git_commit:处理了GPS的问题[2]
	20240617 git_tag:62e368b git_commit:处理了GPS的问题[1]
	20240605 git_tag:a721049 git_commit:加了语音模块的任务版，GPS能跑但有bug
	20240529 git_tag:baf546e git_commit:温湿度加速度计任务版
	20240518 git_tag:0d13e68 git_commit:初赛代码（原始版本）
	### 截至区域赛初赛
	
/kernel_managed：
	该目录为支持软件正常运行的Linux Kernel及dtb等系统固件，备赛初期已确定好，后续无需更新，故不作更新。
	