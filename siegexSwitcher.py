"""
Rainbow Six Siege X Server Switcher
Copyright (c) 2025 0xsweat
Licensed under the MIT License. See LICENSE file in the project root for full license information.

Author: 0xsweat
Date: 6/23/2025

The purpose of this script is to change the user's Rainbow Six Siege matchmaking server.
"""
import os
import argparse

data_dir: str = f"C:/Users/{os.getlogin()}/Documents/My Games/Rainbow Six - Siege"
#data_dir: str = f"C:/Users/{os.getlogin()}/Desktop/projects/rs"

servers: list[str] = [
    "default", # lowest ping
    "playfab/australiaeast",
    "playfab/brazilsouth",
    "playfab/centralus",
    "playfab/eastasia",
    "playfab/eastus",
    "playfab/japaneast",
    "playfab/northeurope",
    "playfab/southafricanorth",
    "playfab/southcentralus",
    "playfab/southeastasia",
    "playfab/uaenorth",
    "playfab/westeurope",
    "playfab/westus",
]

def configs_path(data_dir: str) -> list[str]:
    """
    Returns a list of paths to the config files in the specified data directory.
    """
    configs: list[str] = []
    for x in os.listdir(data_dir):
        if os.path.isdir(f"{data_dir}/{x}") and "GameSettings.ini" in os.listdir(f"{data_dir}/{x}"):
            configs.append(f"{data_dir}/{x}/GameSettings.ini")
    if configs:
        print("Config(s) found!")
        for y,x in enumerate(configs):
            with open(x, "r") as f:
                server = ''.join(
                    [i for i in f.read().split("\n") if i.startswith("DataCenterHint")]
                )
            print(f" {y + 1}. {x.split('/')[-2]} : {server.split('=')[1].replace('playfab/', '')}")
    return configs

def get_servers() -> list[str]:
    """
    Returns a list of available servers.
    """
    print("Available servers:")
    for y, x in enumerate(servers):
        print(f"{y + 1}. {x.replace('playfab/', '')}")
    return servers

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Change Rainbow Six Siege matchmaking server.", epilog="by 0xsweat 6/23/2025")
    parser.add_argument("--data-dir", "-d", type=str, default=data_dir, help="Path to the game data directory.")
    parser.add_argument("--servers", action="store_true", help="Display the list of available servers and exit.")
    parser.add_argument("--version", action="version", version="1.0.0",
                        help="Show the version of the script and exit.")
    parser.add_argument("--server", "-s", type=str, choices=[x.replace("playfab/", "") for x in servers],
                         help="Specify a server to set as the matchmaking server.")
    parser.add_argument("--configs", action="store_true",
                        help="Display the list of config files and their current server and exit.")
    args = parser.parse_args()
    if args.servers:
        get_servers()
        exit()
    if args.configs:
        configs = configs_path(args.data_dir)
        if not configs:
            print("No config files found.")
        exit()
    if not os.path.exists(args.data_dir):
        print(f"Couldn't find your game configs :(")
        exit()
    configs: list[str] = configs_path(args.data_dir)
    if not configs:
        print("Couldn't find any config files :(")
        exit()
    print("Please select a server :")
    for i, server in enumerate(servers):
        print(f"{i + 1}. {server.replace('playfab/', '')}")
    choice: int = int(input("Enter the number of your choice: "))
    if choice < 1 or choice > len(servers):
        print("Invalid choice. Exiting.")
        exit()
    selected_server = servers[choice - 1]
    for x in configs:
        with open(x, "r") as f:
            lines = f.readlines()
        with open(x, "w") as f:
            for line in lines:
                if line.startswith("DataCenterHint"):
                    f.write(f"DataCenterHint={selected_server}\n")
                else:
                    f.write(line)