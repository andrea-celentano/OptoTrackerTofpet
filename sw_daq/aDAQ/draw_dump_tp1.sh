PREFIX=${1}
echo $PREFIX
D=$(dirname ${PREFIX}.raw3)
P=$(basename ${PREFIX})

echo $D
echo $P

mkdir -p ${D}/pdf
mkdir -p ${D}/rpt

aDAQ/buildRaw ${D}/${P} ${D}/${P}.root

root -l -q ${D}/${P}.root aDAQ/draw_dump_tp1.C
mv str.root ${D}/rpt/${P}_str.root
for f in str_*; do
	STR=$(basename ${f} .txt)
	cp ${f} str.txt
	mv ${f} ${D}/rpt/${P}_${f}
 	root -l -q aDAQ/gDraw2D_tp1.C
 	mv graph2d.pdf ${D}/pdf/${P}_${STR}.pdf
done

