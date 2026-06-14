import os
import re
import time
import numpy as np
import requests
from PIL import Image
from io import BytesIO
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

POKEMON_IDS = list(range(1, 152))
IMG_W = 96
IMG_H = 96
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
OUTPUT_DIR = os.path.join(PROJECT_ROOT, "pokemons")
MOVE_CACHE = {}
FALLBACK_MOVES = {
    "weak": {"name": "Tackle", "power": 35, "category": "weak"},
    "medium": {"name": "Body Slam", "power": 60, "category": "medium"},
    "strong": {"name": "Hyper Beam", "power": 90, "category": "strong"},
    "special": {"name": "Swift", "power": 60, "category": "special"},
}


def make_session():
    session = requests.Session()
    retry = Retry(
        total=5,
        backoff_factor=1,
        status_forcelist=[429, 500, 502, 503, 504],
        allowed_methods=["GET"],
    )
    adapter = HTTPAdapter(max_retries=retry)
    session.mount("https://", adapter)
    session.mount("http://", adapter)
    return session


SESSION = make_session()


def api_get(url, timeout=30):
    for attempt in range(1, 4):
        try:
            resp = SESSION.get(url, timeout=timeout)
            if resp.status_code == 200:
                return resp
            print(f"    HTTP {resp.status_code} — tentativa {attempt}/3")
        except requests.exceptions.ReadTimeout:
            print(f"    Timeout — tentativa {attempt}/3, aguardando {attempt * 3}s...")
            time.sleep(attempt * 3)
        except requests.exceptions.ConnectionError:
            print(f"    Erro de conexão — tentativa {attempt}/3, aguardando {attempt * 3}s...")
            time.sleep(attempt * 3)
    return None


def get_move_detail(move_url):
    if move_url in MOVE_CACHE:
        return MOVE_CACHE[move_url]

    resp = api_get(move_url)
    if not resp:
        return None

    data = resp.json()
    detail = {
        "name": data["name"].replace("-", " ").title(),
        "power": data.get("power"),
        "damage_class": data["damage_class"]["name"],
    }
    MOVE_CACHE[move_url] = detail
    time.sleep(0.2)
    return detail


def classify_move(detail):
    power = detail["power"]
    damage_class = detail["damage_class"]

    if damage_class == "status" or power is None:
        return None

    if damage_class == "special":
        return "special"
    if power <= 40:
        return "weak"
    if power <= 80:
        return "medium"
    return "strong"


def get_moves_by_category(pokemon_data):
    slots = {"weak": None, "medium": None, "strong": None, "special": None}
    level_up_moves = [
        move for move in pokemon_data["moves"]
        if any(v["move_learn_method"]["name"] == "level-up" for v in move["version_group_details"])
    ]
    all_moves = level_up_moves + [m for m in pokemon_data["moves"] if m not in level_up_moves]

    for move_entry in all_moves:
        if all(slots.values()):
            break

        detail = get_move_detail(move_entry["move"]["url"])
        if not detail:
            continue

        category = classify_move(detail)
        if not category or slots[category] is not None:
            continue

        slots[category] = {
            "name": detail["name"],
            "power": detail["power"],
            "category": category,
        }
        print(f"    [{category:7s}] {detail['name']} (power:{detail['power']})")

    result = []
    for category in ["weak", "medium", "strong", "special"]:
        if slots[category] is None:
            print(f"    [{category:7s}] usando fallback")
            result.append(FALLBACK_MOVES[category])
        else:
            result.append(slots[category])

    return result


def get_stats(pokemon_data):
    stats = {s["stat"]["name"]: s["base_stat"] for s in pokemon_data["stats"]}
    return {
        "hp": stats.get("hp", 45),
        "attack": stats.get("attack", 45),
        "defense": stats.get("defense", 40),
    }


def get_types(pokemon_data):
    return [t["type"]["name"] for t in pokemon_data["types"]]


def sanitize(name):
    return re.sub(r"[^a-z0-9_-]", "_", name.lower())


def fix_colors(img):
    pixels = np.array(img, dtype=np.uint16)
    rgb = pixels[:, :, :3]
    alpha = pixels[:, :, 3:] if pixels.shape[2] == 4 else None

    r = (rgb[:, :, 0] >> 3) & 0x1F
    g = (rgb[:, :, 1] >> 2) & 0x3F
    b = (rgb[:, :, 2] >> 3) & 0x1F
    r8 = (r << 3) | (r >> 2)
    g8 = (g << 2) | (g >> 4)
    b8 = (b << 3) | (b >> 2)

    corrected_rgb = np.stack([r8, g8, b8], axis=2).astype(np.uint8)
    if alpha is not None:
        corrected = np.concatenate([corrected_rgb, alpha.astype(np.uint8)], axis=2)
        return Image.fromarray(corrected, mode="RGBA")
    return Image.fromarray(corrected_rgb, mode="RGB")


def download_image(url):
    if not url:
        return None

    resp = api_get(url)
    if not resp:
        return None

    img = Image.open(BytesIO(resp.content))
    img = img.convert("RGBA")
    img = img.resize((IMG_W, IMG_H), Image.LANCZOS)
    return fix_colors(img)


def save_data_txt(path, name, stats, types, moves):
    with open(path, "w") as f:
        f.write(f"NAME:{name}\n")
        f.write(f"TYPE1:{types[0] if len(types) > 0 else ''}\n")
        f.write(f"TYPE2:{types[1] if len(types) > 1 else ''}\n")
        f.write(f"HP:{stats['hp']}\n")
        f.write(f"ATK:{stats['attack']}\n")
        f.write(f"DEF:{stats['defense']}\n")
        for i, move in enumerate(moves, 1):
            f.write(f"MOVE{i}:{move['name']}|{move['power']}|{move['category']}\n")


def process_pokemon(pid):
    resp = api_get(f"https://pokeapi.co/api/v2/pokemon/{pid}")
    if not resp:
        print(f"  ✗ Falha ao buscar #{pid} após todas as tentativas")
        return False

    data = resp.json()
    name = data["name"]
    stats = get_stats(data)
    types = get_types(data)

    print(f"  Nome : {name.capitalize()}")
    print(f"  Tipos: {' / '.join(types)}")
    print(f"  Stats: HP:{stats['hp']} ATK:{stats['attack']} DEF:{stats['defense']}")
    print("  Buscando ataques...")

    moves = get_moves_by_category(data)
    folder_path = os.path.join(OUTPUT_DIR, f"{pid:03d}_{sanitize(name)}")
    os.makedirs(folder_path, exist_ok=True)

    sprites = data["sprites"]
    front_img = download_image(sprites.get("front_default"))
    back_img = download_image(sprites.get("back_default"))

    if front_img:
        front_img.save(os.path.join(folder_path, "front.png"), "PNG")
        print(f"  Sprite frente: {IMG_W}x{IMG_H} OK")
    else:
        print("  ⚠ Sem sprite frente")

    if back_img:
        back_img.save(os.path.join(folder_path, "back.png"), "PNG")
        print(f"  Sprite costas: {IMG_W}x{IMG_H} OK")
    else:
        print("  ⚠ Sem sprite costas")

    save_data_txt(os.path.join(folder_path, "data.txt"), name, stats, types, moves)
    print(f"  ✔ Salvo em {folder_path}/")
    return True


if __name__ == "__main__":
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    print(f"Baixando {len(POKEMON_IDS)} pokémons da PokeAPI...")
    print(f"Imagens: {IMG_W}x{IMG_H} PNG\n")

    ok = err = 0
    for pid in POKEMON_IDS:
        print("=" * 45)
        print(f"Pokémon #{pid:03d}...")
        try:
            if process_pokemon(pid):
                ok += 1
            else:
                err += 1
        except Exception as e:
            print(f"  ✗ Erro inesperado: {e}")
            err += 1
        time.sleep(0.5)

    print("\n" + "=" * 45)
    print(f"Concluído! {ok} ok  |  {err} erros")
    print(f"Copie a pasta '{OUTPUT_DIR}/' para a raiz do cartão SD.")
