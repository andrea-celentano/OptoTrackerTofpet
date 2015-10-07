PREFIX=${1}
echo $PREFIX
D=$(dirname ${PREFIX}.raw3)
P=$(basename ${PREFIX})

echo "D IS" $D
echo "P IS" $P

mkdir -p ${D}/pdf
mkdir -p ${D}/rpt

#aDAQ/buildRaw ${D}/${P} ${D}/${P}.root

#root -l -q -b ${D}/${P}.root aDAQ/draw_dump_tp1.C
#mv str.root ${D}/rpt/${P}_str.root
for f in ${D}/rpt/${P}_str_*; do
        echo "File is: " $f
	STR=$(basename ${f} .txt)
	cp ${f} str.txt
#	mv ${f} ${D}/rpt/${P}_${f}
 	root -l -q -b aDAQ/gDraw2D_tp1.C
 	mv graph2d.pdf ${D}/pdf/${STR}.pdf
done

