install:
	dtc -I dts -O dtb -o BB-MAC-DISPLAY-00A0.dtbo -@ BB-MAC-DISPLAY-00A0.dts
	cp ./BB-MAC-DISPLAY-00A0.dtbo /lib/firmware/BB-MAC-DISPLAY-00A0.dtbo
    gcc macdisplay.c -lprussdrv -lpthread -o macdisplay
	pasm -b pru_0.p
uninstall:
	rm ./macdisplay ./pru_0.bin ./BB-MAC-DISPLAY-00A0.dtbo