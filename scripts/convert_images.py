"""
convert_images.py — Converte imagens para o display ILI9341 (320x240)

Estrutura esperada:
  images/
    background.png  (ou .jpg, .jpeg, .webp, .bmp)
    login.png
    pick.png
    results.png
    trainer_1.png
    trainer_2.png

Saída (pasta components/):
  components/
    background.jpg
    login.jpg
    pick.jpg
    results.jpg
    trainer_1.jpg
    trainer_2.jpg

Como usar:
  pip install pillow numpy
  python convert_images.py
"""

import os
import numpy as np
from PIL import Image

# =====================================================
# CONFIGURAÇÕES
# =====================================================

DISPLAY_W  = 320
DISPLAY_H  = 240
QUALITY    = 92
INPUT_DIR  = "images"
OUTPUT_DIR = "components"

# Modo de redimensionamento: "contain" (sem corte), "cover" (corta), "stretch" (distorce)
RESIZE_MODE = "stretch"  # Mude para "cover" se quiser cortar, ou "contain" para bordas pretas

IMAGES = {
    "background": "background",
    "login":      "login",
    "pick":       "pick",
    "results":    "results",
    "trainer_1":  "trainer_1",
    "trainer_2":  "trainer_2",
}

EXTENSIONS = [".png", ".jpg", ".jpeg", ".webp", ".bmp"]

# =====================================================
# REMOVE BORDAS PRETAS/BRANCAS AUTOMÁTICO
# Detecta e corta faixas sólidas nas bordas da imagem
# =====================================================
def crop_borders(img, threshold=20):
    """
    Remove bordas sólidas (pretas ou brancas) da imagem.
    threshold: tolerância de cor (0=exato, 20=permite variação leve)
    """
    pixels = np.array(img)
    h, w   = pixels.shape[:2]

    def is_solid_row(row):
        # Verifica se a linha é quase toda preta ou quase toda branca
        mean = row.mean(axis=0)  # média RGB da linha
        is_black = np.all(mean < threshold)
        is_white = np.all(mean > 255 - threshold)
        return is_black or is_white

    def is_solid_col(col):
        mean = col.mean(axis=0)
        is_black = np.all(mean < threshold)
        is_white = np.all(mean > 255 - threshold)
        return is_black or is_white

    # Encontra bordas top/bottom
    top    = 0
    bottom = h
    left   = 0
    right  = w

    for y in range(h):
        if not is_solid_row(pixels[y]):
            top = y
            break

    for y in range(h - 1, -1, -1):
        if not is_solid_row(pixels[y]):
            bottom = y + 1
            break

    for x in range(w):
        if not is_solid_col(pixels[:, x]):
            left = x
            break

    for x in range(w - 1, -1, -1):
        if not is_solid_col(pixels[:, x]):
            right = x + 1
            break

    # Só corta se removeu algo significativo (mais de 2px)
    if top > 2 or left > 2 or bottom < h - 2 or right < w - 2:
        cropped = img.crop((left, top, right, bottom))
        print(f"    Bordas removidas: top={top} bottom={h-bottom} left={left} right={w-right}")
        return cropped

    return img

# =====================================================
# CORREÇÃO DE COR RGB565
# =====================================================
def fix_colors_rgb565(img):
    pixels = np.array(img, dtype=np.uint16)

    r = (pixels[:,:,0] >> 3) & 0x1F
    g = (pixels[:,:,1] >> 2) & 0x3F
    b = (pixels[:,:,2] >> 3) & 0x1F

    r8 = ((r << 3) | (r >> 2)).astype(np.uint8)
    g8 = ((g << 2) | (g >> 4)).astype(np.uint8)
    b8 = ((b << 3) | (b >> 2)).astype(np.uint8)

    return Image.fromarray(np.stack([r8, g8, b8], axis=2))

# =====================================================
# CONVERTE UMA IMAGEM
# =====================================================
def convert(input_path, output_path):
    img = Image.open(input_path)

    # Trata transparência
    if img.mode == "RGBA":
        bg  = Image.new("RGBA", img.size, (0, 0, 0, 255))
        img = Image.alpha_composite(bg, img).convert("RGB")
    else:
        img = img.convert("RGB")

    original_w, original_h = img.size

    # Remove bordas sólidas antes de redimensionar
    img = crop_borders(img)
    cropped_w, cropped_h = img.size

    # Se a imagem já é exatamente 320x240 após o crop, usa direto
    if cropped_w == DISPLAY_W and cropped_h == DISPLAY_H:
        canvas = img
    elif RESIZE_MODE == "stretch":
        # Stretch: redimensiona distorcendo se necessário (sem cortes, sem bordas)
        canvas = img.resize((DISPLAY_W, DISPLAY_H), Image.LANCZOS)
    elif RESIZE_MODE == "cover":
        # Cover: escala para cobrir e corta o excesso
        ratio_w = DISPLAY_W / cropped_w
        ratio_h = DISPLAY_H / cropped_h
        ratio   = max(ratio_w, ratio_h)

        new_w = int(cropped_w * ratio)
        new_h = int(cropped_h * ratio)
        img   = img.resize((new_w, new_h), Image.LANCZOS)

        left   = (new_w - DISPLAY_W) // 2
        top    = (new_h - DISPLAY_H) // 2
        canvas = img.crop((left, top, left + DISPLAY_W, top + DISPLAY_H))
    else:  # contain
        # Contain: escala mantendo proporção e preenche fundo com preto
        ratio_w = DISPLAY_W / cropped_w
        ratio_h = DISPLAY_H / cropped_h
        ratio   = min(ratio_w, ratio_h)

        new_w = int(cropped_w * ratio)
        new_h = int(cropped_h * ratio)
        img   = img.resize((new_w, new_h), Image.LANCZOS)

        canvas = Image.new("RGB", (DISPLAY_W, DISPLAY_H), (0, 0, 0))
        left   = (DISPLAY_W - new_w) // 2
        top    = (DISPLAY_H - new_h) // 2
        canvas.paste(img, (left, top))

    # Corrige cores para o ILI9341
    canvas = fix_colors_rgb565(canvas)

    # Salva como JPEG
    canvas.save(output_path, "JPEG", quality=QUALITY)

    size_kb = os.path.getsize(output_path) / 1024
    print(f"  ✔ {os.path.basename(input_path)} ({original_w}x{original_h})"
          f" → {os.path.basename(output_path)} ({DISPLAY_W}x{DISPLAY_H})"
          f" [{size_kb:.1f} KB]")

# =====================================================
# ENCONTRA O ARQUIVO DE ENTRADA
# =====================================================
def find_input(name):
    for ext in EXTENSIONS:
        path = os.path.join(INPUT_DIR, name + ext)
        if os.path.exists(path):
            return path
    return None

# =====================================================
# MAIN
# =====================================================
if __name__ == "__main__":
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    if not os.path.exists(INPUT_DIR):
        os.makedirs(INPUT_DIR)
        print(f"Pasta '{INPUT_DIR}/' criada.")
        print(f"Coloque suas imagens lá e rode novamente.")
        print(f"\nImagens esperadas:")
        for out_name, in_name in IMAGES.items():
            print(f"  {INPUT_DIR}/{in_name}.png  →  {OUTPUT_DIR}/{out_name}.jpg")
        exit(0)

    print(f"Convertendo imagens de '{INPUT_DIR}/' para '{OUTPUT_DIR}/'")
    print(f"Resolução: {DISPLAY_W}x{DISPLAY_H} | Qualidade JPEG: {QUALITY}\n")

    ok = err = 0

    for out_name, in_name in IMAGES.items():
        input_path  = find_input(in_name)
        output_path = os.path.join(OUTPUT_DIR, out_name + ".jpg")

        if not input_path:
            print(f"  ✗ {in_name} não encontrado em '{INPUT_DIR}/'")
            err += 1
            continue

        try:
            print(f"  → {in_name}...")
            convert(input_path, output_path)
            ok += 1
        except Exception as e:
            print(f"  ✗ Erro ao converter {in_name}: {e}")
            err += 1

    print(f"\n{'='*50}")
    print(f"Concluído! {ok} ok  |  {err} erros")

    if ok > 0:
        print(f"\nCopie a pasta '{OUTPUT_DIR}/' para a raiz do cartão SD.")
        print(f"\nNo ESP32, carregue assim:")
        print(f'  drawJpegFromSD("/components/background.jpg", 0, 0);')
        print(f'  drawJpegFromSD("/components/login.jpg",      0, 0);')
        print(f'  drawJpegFromSD("/components/pick.jpg",       0, 0);')
        print(f'  drawJpegFromSD("/components/results.jpg",    0, 0);')
        print(f'  drawJpegFromSD("/components/trainer_1.jpg",  0, 0);')
        print(f'  drawJpegFromSD("/components/trainer_2.jpg",  0, 0);')
