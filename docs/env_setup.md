# 編譯與燒錄
## 在WSL編譯與
WSL選擇安裝Ubuntu的系統後，安裝Arm Toolchain & build essential:

```shell
# gcc-arm-none-eabi = None os + embedded ABI
$ sudo apt update && sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi build-essential
```

成功後，可透過`make`來編譯以及build image。編譯後，再將檔案給複製回原生的Windows上
這個路徑要記得，後面debugger也需要使用到編譯好的firmware.elf
```shell
$ make clean && make
# Copy files to D:/your/folder
$ cp bin/* /mnt/d/your/folder
```

## Windows環境架設

到STM官方網站，下載 & 安裝`STM32CubeCLT` & `STM32CubeProg`。這樣會安裝Debugger與燒錄用的工具
安裝好後，可以透過`STM32_Programmer_CLI`工具來進行燒錄:
```shell
# Program firmware.bin to address 0x0800_0000
$ STM32_Programmer_CLI -c port=SWD -w firmware.bin 0x08000000 -v
```

並且，STM提供gdb server，用來連接ST-Link & GNU GDB。透過以下指令，開啟gdb server等候gdb連線:
```shell
$ cd \path\to\stlink-gdb-server
$ ST-LINK_gdbserver.exe -d -cp \path\to\STM32CubeProgrammer\bin
STMicroelectronics ST-LINK GDB server. Version 7.14.0
Copyright (c) 2026, STMicroelectronics. All rights reserved.

Starting server with the following options:
        Persistent Mode            : Disabled
        Logging Level              : 1
        Listen Port Number         : 61234
        Status Refresh Delay       : 15s
        Verbose Mode               : Disabled
        SWD Debug                  : Enabled

ST-LINK firmware update available. Upgrade the ST-LINK firmware using the upgrade tool.
Waiting for debugger connection...
```

此時，特別注意，輸出內容的 `Listen Port Number`。以此例而言，他會監聽Localhost:61234。也因此，後續debugger連線的時候，就要先連線到這個Port，才能與板子溝通:
```shell
# Cd to folder contains your image files
$ cd D:\your\folder
$ arm-none-eabi-gdb.exe
(gdb) target extended-remote localhost:61234
(gdb) file firmware.elf
```

後面就是通用的gdb指令，例如 `monitor reset`來回到reset狀態、`info registers`來看到CPU registers、`break main`來在main function打上一個break point。

