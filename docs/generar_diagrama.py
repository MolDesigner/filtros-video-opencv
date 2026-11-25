import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Polygon
fig, ax = plt.subplots(figsize=(13, 8.6)); ax.set_xlim(0,130); ax.set_ylim(-14,74); ax.axis("off")
C_ABS="#E8EEF7"; C_CON="#FFFFFF"; C_MAIN="#FFF4E0"; EDGE="#33475B"
def caja(x,y,w,h,titulo,miembros=(),color=C_CON,abstracta=False,fs=10,estereotipo=None):
    ax.add_patch(Rectangle((x,y),w,h,fc=color,ec=EDGE,lw=1.3))
    if abstracta: estereotipo="«abstracta»"
    th=(5.4 if estereotipo else 4.2) if miembros else h
    ax.text(x+w/2,y+h-th/2,(estereotipo+"\n" if estereotipo else "")+titulo,ha="center",va="center",
            fontsize=fs if not estereotipo else 9.5,weight="bold",style="italic" if abstracta else "normal")
    if miembros:
        ax.plot([x,x+w],[y+h-th]*2,color=EDGE,lw=1)
        ax.text(x+1,y+h-th-0.8,"\n".join(miembros),ha="left",va="top",fontsize=7.6,family="monospace")
    return (x,y,w,h)
def herencia(xs,ys,xd,yd):  # flecha con triángulo hueco en (xd,yd), apuntando hacia arriba
    ax.plot([xs,xs],[ys,ys+ (yd-ys)/2],color=EDGE,lw=1.1)
def triangulo(x,y):
    ax.add_patch(Polygon([[x,y],[x-1.4,y-2.2],[x+1.4,y-2.2]],closed=True,fc="white",ec=EDGE,lw=1.1))
def rombo(x,y,dx,dy):  # rombo relleno (composición) en el dueño
    import math; L=math.hypot(dx,dy); ux,uy=dx/L,dy/L; px,py=-uy,ux
    pts=[[x,y],[x+ux*1.6+px*0.8,y+uy*1.6+py*0.8],[x+ux*3.2,y+uy*3.2],[x+ux*1.6-px*0.8,y+uy*1.6-py*0.8]]
    ax.add_patch(Polygon(pts,closed=True,fc=EDGE,ec=EDGE))
def comp(x1,y1,x2,y2,etq="",dx=0,dy=0):
    ax.plot([x1,x2],[y1,y2],color=EDGE,lw=1.1); rombo(x1,y1,x2-x1,y2-y1)
    if etq: ax.text((x1+x2)/2+dx,(y1+y2)/2+dy,etq,fontsize=7.5,color="#555",ha="center")

# Aplicación
caja(44,52,34,17,"Aplicacion",["+ ejecutar()","- manejarTecla(int)","- dibujarInterfaz(Mat&)","- controles_: map<int,function>"],C_MAIN)
caja(3,56,27,12,"Configuracion",["parsearArgumentos(argc,argv)","textoUso(programa)"])
ax.annotate("",xy=(30,62),xytext=(44,62),arrowprops=dict(arrowstyle="->",ls="--",color=EDGE)); ax.text(37,63,"usa",fontsize=7.5,color="#555",ha="center")
caja(96,58,30,11,"Ventana  (RAII)",["mostrar(), esperarTecla()","estaAbierta()"])
caja(96,42,30,11,"MedidorFPS",["registrarCuadro()","fps(): double  (chrono)"])
comp(78,64,96,64); comp(78,55,96,48)
# Fuentes
caja(4,30,34,12,"FuenteVideo",["+ leer(Mat&): bool","+ descripcion(), retardoMs()"],C_ABS,True)
comp(50,52,30,42,"unique_ptr",-6,0)
caja(0,10,22.5,10,"FuenteOpenCV",["# captura_:","  VideoCapture"],C_ABS,True)
caja(24.5,10,21,11,"FuenteEnHilo",["- hilo_: std::thread","- detener_: atomic"],estereotipo="«decorador»")
ax.plot([11.25,11.25,35,35],[20,24.5,24.5,21],color=EDGE,lw=1.1); ax.plot([21,21],[24.5,27.8],color=EDGE,lw=1.1); triangulo(21,30)
# FuenteEnHilo envuelve otra FuenteVideo (interna_)
ax.plot([44,44,38],[24.2,36,36],color=EDGE,lw=1.1); rombo(44,21,0,1)
ax.text(45,28,"interna_",fontsize=7.5,color="#555",rotation=90,va="center")
caja(0,-12,10,8,"Webcam",fs=9); caja(11.5,-12,13,8,"ArchivoVideo",fs=8.5)
ax.plot([5,5,18,18],[-4,0,0,-4],color=EDGE,lw=1.1); ax.plot([11.25,11.25],[0,7.8],color=EDGE,lw=1.1); triangulo(11.25,10)
caja(25,-12,22,9,"ColaSegura<Mat>",["insertar(), extraer()","mutex + cond. var."])
comp(35,10,35,-3)
# Filtros
caja(50,30,32,11,"CadenaFiltros",["agregar(), alternar(tecla)","procesar(entrada, salida)"])
comp(64,52,64,41)
caja(94,28,32,12,"Filtro",["+ aplicar(entrada, salida)","+ nombre()"],C_ABS,True)
comp(82,35.5,94,35.5,"1..*  unique_ptr",0,1)
nombres=["Grises","Desenfoque","Canny","Sepia","Negativo","Pixelado","Relieve"]
x0=47; w=10.6; paso=11.4; centros=[]
for i,n in enumerate(nombres):
    x=x0+i*paso; caja(x,4,w,8,"Filtro\n"+n,fs=8.5); centros.append(x+w/2)
ax.plot([centros[0],centros[-1]],[17,17],color=EDGE,lw=1.1)
for c in centros: ax.plot([c,c],[12,17],color=EDGE,lw=1.1)
ax.plot([110,110],[17,25.8],color=EDGE,lw=1.1); triangulo(110,28)
plt.savefig("diagrama_clases.png",dpi=170,bbox_inches="tight",facecolor="white")
