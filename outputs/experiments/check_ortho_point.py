import cv2


def draw_circle(ortho, point, color):
    cv2.circle(ortho, center=point, radius=5, color=color, thickness=-1)


def main():
    ortho = cv2.imread("./ortho.tif")
    draw_circle(ortho, (int(1186.8082), int(938.50433)), (255, 255, 0))
    draw_circle(ortho, (int(1138.1256), int(1027.8319)), (0, 255, 0))

    draw_circle(ortho, (int(1142.4763), int(1025.5763)), (0, 0, 255))
    draw_circle(ortho, (int(1189.8596), int(938.52997)), (255, 0, 0))

    ortho_view = cv2.resize(ortho, dsize=None, fx=0.5, fy=0.5)
    cv2.imshow("", ortho_view)
    cv2.waitKey(0)


if __name__ == "__main__":
    main()
