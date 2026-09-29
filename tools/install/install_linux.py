#!/usr/bin/env python3
"""Install AlphaRing for Halo: The Master Chief Collection on Linux (Steam Deck, Batocera, desktop Steam).

    python3 install_linux.py              # latest release from GitHub
    python3 install_linux.py WTSAPI32.dll # a DLL you already have
    python3 install_linux.py --dry-run    # show what it would do
    python3 install_linux.py --uninstall  # put back what it replaced
    python3 install_linux.py --skip-eac-prompt  # also start MCC without its EAC launcher prompt

It finds MCC in your Steam libraries, backs up and replaces MCC's WTSAPI32.dll, and adds
WINEDLLOVERRIDES="WTSAPI32=n,b" to MCC's launch options. Steam rewrites its config when it exits, so the launch
options are only changed while Steam is closed; otherwise it tells you what to paste. Only the Python standard
library is used.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import urllib.request

APP_ID = "976730"
REPO = "kirklandsig/AlphaRing"
OVERRIDE = 'WINEDLLOVERRIDES="WTSAPI32=n,b"'
# Runs the game itself instead of mcclauncher.exe (which asks whether to use EAC; AlphaRing needs it off).
SKIP_LAUNCHER = "bash -c 'exec \"${@/mcclauncher.exe/MCC/Binaries/Win64/MCC-Win64-Shipping.exe}\"' --"
STEAM_ROOTS = [
    "/userdata/system/add-ons/steam/.local/share/Steam",                  # Batocera add-on
    "~/.local/share/Steam",                                               # Steam Deck, most distros
    "~/.steam/steam",
    "~/.steam/root",
    "~/.var/app/com.valvesoftware.Steam/.local/share/Steam",              # Flatpak
    "~/snap/steam/common/.local/share/Steam",                             # Snap
]


# --- Steam's text VDF (KeyValues) -------------------------------------------------------------------------------

def vdf_tokens(text):
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c.isspace():
            i += 1
        elif c == "/" and text.startswith("//", i):
            i = text.find("\n", i)
            i = n if i < 0 else i
        elif c in "{}":
            yield c
            i += 1
        elif c == '"':
            j, out = i + 1, []
            while j < n and text[j] != '"':
                if text[j] == "\\" and j + 1 < n:
                    out.append({"n": "\n", "t": "\t", "\\": "\\", '"': '"'}.get(text[j + 1], text[j + 1]))
                    j += 2
                else:
                    out.append(text[j])
                    j += 1
            yield ("s", "".join(out))
            i = j + 1
        else:  # unquoted token
            j = i
            while j < n and not text[j].isspace() and text[j] not in '{}"':
                j += 1
            yield ("s", text[i:j])
            i = j


def vdf_load(text):
    """-> list of (key, value) pairs, value a str or such a list (keeps order and duplicate keys)."""
    tokens = vdf_tokens(text)

    def block():
        items = []
        for tok in tokens:
            if tok == "}":
                return items
            key = tok[1]
            val = next(tokens)
            items.append((key, block() if val == "{" else val[1]))
        return items

    return block()


def vdf_escape(s):
    return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")


def vdf_dump(items, depth=0):
    out, tab = [], "\t" * depth
    for key, val in items:
        if isinstance(val, list):
            out.append(f'{tab}"{vdf_escape(key)}"\n{tab}{{\n{vdf_dump(val, depth + 1)}{tab}}}\n')
        else:
            out.append(f'{tab}"{vdf_escape(key)}"\t\t"{vdf_escape(val)}"\n')
    return "".join(out)


def vdf_get(items, *path):
    for name in path:
        items = next((v for k, v in items if k.lower() == name.lower()), None)
        if items is None:
            return None
    return items


def vdf_child(items, name):
    """The block `name` under items, created when missing."""
    found = vdf_get(items, name)
    if isinstance(found, list):
        return found
    block = []
    items.append((name, block))
    return block


# --- finding MCC ----------------------------------------------------------------------------------------------

def steam_roots():
    seen = []
    for root in STEAM_ROOTS:
        path = os.path.realpath(os.path.expanduser(root))
        if os.path.isdir(os.path.join(path, "steamapps")) and path not in seen:
            seen.append(path)
    return seen


def load_vdf(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return vdf_load(f.read())


def libraries(root):
    libs = [root]
    vdf = os.path.join(root, "steamapps", "libraryfolders.vdf")
    if os.path.exists(vdf):
        data = load_vdf(vdf)
        for _, entry in vdf_get(data, "libraryfolders") or []:
            if isinstance(entry, list):
                path = vdf_get(entry, "path")
                if path and os.path.isdir(path) and os.path.realpath(path) not in map(os.path.realpath, libs):
                    libs.append(path)
    return libs


def find_mcc():
    for root in steam_roots():
        for lib in libraries(root):
            acf = os.path.join(lib, "steamapps", f"appmanifest_{APP_ID}.acf")
            if not os.path.exists(acf):
                continue
            installdir = vdf_get(load_vdf(acf), "AppState", "installdir")
            win64 = os.path.join(lib, "steamapps", "common", installdir or "Halo The Master Chief Collection",
                                 "MCC", "Binaries", "Win64")
            if os.path.isdir(win64):
                return root, win64
    return None, None


# --- the DLL ----------------------------------------------------------------------------------------------------

def latest_release_dll(dest):
    req = urllib.request.Request(f"https://api.github.com/repos/{REPO}/releases",
                                 headers={"Accept": "application/vnd.github+json", "User-Agent": "alpharing-installer"})
    releases = json.load(urllib.request.urlopen(req, timeout=30))
    for release in releases:  # newest first, pre-releases included (every AlphaRing release is one)
        for asset in release.get("assets", []):
            if asset["name"].lower() == "wtsapi32.dll":
                print(f"Downloading {release['tag_name']} ...")
                with urllib.request.urlopen(asset["browser_download_url"], timeout=120) as r, open(dest, "wb") as f:
                    shutil.copyfileobj(r, f)
                return release["tag_name"]
    raise SystemExit("No release with a WTSAPI32.dll found on GitHub.")


def is_pe(path):
    with open(path, "rb") as f:
        return f.read(2) == b"MZ"


def copy_whole(source, dest):
    """dest ends up a full copy of source or unchanged, never half-written"""
    shutil.copy2(source, dest + ".partial")
    os.replace(dest + ".partial", dest)


def is_alpharing(path):
    """an AlphaRing build (its log and config names are in it), not the game's or another mod's WTSAPI32.dll"""
    with open(path, "rb") as f:
        data = f.read()
    return data[:2] == b"MZ" and (b"alpha_ring" in data or b"alpharing" in data)


# --- launch options -----------------------------------------------------------------------------------------------

def steam_running():
    try:
        out = subprocess.run(["ps", "-eo", "comm"], capture_output=True, text=True).stdout.split()
    except OSError:
        return False
    return any(name in ("steam", "steamwebhelper") for name in out)


def localconfigs(root):
    base = os.path.join(root, "userdata")
    if not os.path.isdir(base):
        return []
    return [os.path.join(base, uid, "config", "localconfig.vdf") for uid in os.listdir(base)
            if os.path.exists(os.path.join(base, uid, "config", "localconfig.vdf"))]


def app_block(data):
    store = vdf_get(data, "UserLocalConfigStore")
    if store is None:
        return None
    software = vdf_child(store, "Software")
    steam = vdf_child(vdf_child(software, "Valve"), "Steam")
    return vdf_child(vdf_child(steam, "apps"), APP_ID)


def with_override(options, skip_launcher=False):
    options = (options or "").strip()
    if "%command%" not in options:
        options = f"%command% {options}".strip()
    if skip_launcher and "mcclauncher.exe" not in options:
        options = options.replace("%command%", f"{SKIP_LAUNCHER} %command%", 1)
    if "WTSAPI32=n,b" not in options:
        # one WINEDLLOVERRIDES only (a second one would replace the first): ours goes into an existing one
        existing = re.search(r'WINEDLLOVERRIDES=(?:"([^"]*)"|(\S*))', options)
        if existing:
            value = existing.group(1) if existing.group(1) is not None else existing.group(2)
            options = options.replace(existing.group(0), f'WINEDLLOVERRIDES="WTSAPI32=n,b;{value}"', 1)
        else:
            options = f"{OVERRIDE} {options}"
    return options


def without_override(options):
    options = (options or "").replace(OVERRIDE, "").replace("WTSAPI32=n,b;", "")
    return " ".join(options.split())


def set_launch_options(root, uninstall, dry, skip_launcher):
    changed = 0
    for path in localconfigs(root):
        data = load_vdf(path)
        block = app_block(data)
        if block is None:
            continue
        old = vdf_get(block, "LaunchOptions") or ""
        new = without_override(old) if uninstall else with_override(old, skip_launcher)
        if new.strip() == "%command%":
            new = ""
        if new == old:
            continue
        print(f"Launch options ({path}):\n    {old!r}\n -> {new!r}")
        if dry:
            continue
        shutil.copy2(path, path + ".alpharing.bak")
        block[:] = [(k, v) for k, v in block if k.lower() != "launchoptions"] + ([("LaunchOptions", new)] if new else [])
        with open(path + ".partial", "w", encoding="utf-8") as f:
            f.write(vdf_dump(data))
        os.replace(path + ".partial", path)
        changed += 1
    return changed


def compat_tool(root):
    config = os.path.join(root, "config", "config.vdf")
    if not os.path.exists(config):
        return None
    data = load_vdf(config)
    mapping = vdf_get(data, "InstallConfigStore", "Software", "Valve", "Steam", "CompatToolMapping", APP_ID)
    return vdf_get(mapping, "name") if isinstance(mapping, list) else None


# --- main ---------------------------------------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("dll", nargs="?", help="a WTSAPI32.dll to install instead of downloading the latest release")
    ap.add_argument("--dry-run", action="store_true", help="show what would change, change nothing")
    ap.add_argument("--uninstall", action="store_true", help="restore the DLL and launch options AlphaRing replaced")
    ap.add_argument("--skip-eac-prompt", action="store_true",
                    help="start the game directly instead of MCC's launcher, which asks about EAC")
    args = ap.parse_args()

    root, win64 = find_mcc()
    if not win64:
        raise SystemExit("Halo: The Master Chief Collection wasn't found in any Steam library.")
    print(f"MCC: {win64}")
    target, backup = os.path.join(win64, "WTSAPI32.dll"), os.path.join(win64, "WTSAPI32.dll.alpharing.bak")

    if args.uninstall:
        # only AlphaRing's own DLL is replaced or removed, so running this twice (or without an install) is harmless
        if os.path.exists(target) and not is_alpharing(target):
            print(f"{target} isn't AlphaRing - left as it is")
        elif os.path.exists(backup):
            print(f"Restoring {backup}")
            if not args.dry_run: shutil.move(backup, target)
        elif os.path.exists(target):
            print(f"Removing {target}")
            if not args.dry_run: os.remove(target)
    else:
        source = args.dll
        if source is None:
            source = os.path.join(win64, "WTSAPI32.dll.download")
            if args.dry_run:
                print("Would download the latest release's WTSAPI32.dll from GitHub")
            else:
                latest_release_dll(source)
        if not args.dry_run:
            if not is_pe(source):
                raise SystemExit(f"{source} isn't a Windows DLL.")
            # what AlphaRing replaces, once: another AlphaRing build isn't kept
            if os.path.exists(target) and not os.path.exists(backup) and not is_alpharing(target):
                copy_whole(target, backup)
            copy_whole(source, target)
            if source.endswith(".download"): os.remove(source)
        print(f"{'Would install' if args.dry_run else 'Installed'} {target}")

    if steam_running():
        print("\nSteam is running, so its config can't be changed safely now. Either close Steam and run this again,"
              "\nor set MCC's launch options yourself (Properties > General > Launch Options):")
        print(f"    {with_override('', args.skip_eac_prompt)}" if not args.uninstall
              else "    (remove WINEDLLOVERRIDES=\"WTSAPI32=n,b\")")
    else:
        n = set_launch_options(root, args.uninstall, args.dry_run, args.skip_eac_prompt)
        print("Launch options already set." if n == 0 and not args.dry_run else "")

    tool = compat_tool(root)
    print(f"Proton for MCC: {tool or 'Steam default'} (Batocera: the README recommends Proton GE 10-15 or newer)")


if __name__ == "__main__":
    main()
