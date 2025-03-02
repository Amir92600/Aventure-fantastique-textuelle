#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>
#include <limits>
#include <map>
#include <memory>
#include <functional>

class Item {
public:
    std::string name;
    int healthBonus, attackBonus, defenseBonus;

    Item(std::string n, int h, int a, int d) : name(n), healthBonus(h), attackBonus(a), defenseBonus(d) {}
};

class Character {
public:
    std::string name;
    int health, maxHealth, attack, defense, experience, level, gold;
    std::vector<std::shared_ptr<Item>> inventory;

    Character(std::string n, int h, int a, int d) : 
        name(n), health(h), maxHealth(h), attack(a), defense(d), experience(0), level(1), gold(0) {}

    void levelUp() {
        level++;
        maxHealth += 10;
        health = maxHealth;
        attack += 2;
        defense += 1;
        std::cout << name << " passe au niveau " << level << " !" << std::endl;
    }

    void addItem(std::shared_ptr<Item> item) {
        inventory.push_back(item);
        maxHealth += item->healthBonus;
        health = std::min(health + item->healthBonus, maxHealth);
        attack += item->attackBonus;
        defense += item->defenseBonus;
        std::cout << "Vous avez obtenu : " << item->name << std::endl;
    }

    void displayInventory() {
        std::cout << "\n--- Inventaire ---" << std::endl;
        if (inventory.empty()) {
            std::cout << "Votre inventaire est vide." << std::endl;
        } else {
            for (const auto& item : inventory) {
                std::cout << "- " << item->name << std::endl;
            }
        }
    }
};

class Monster {
public:
    std::string name;
    int health, attack, defense, experienceReward, goldReward;

    Monster(std::string n, int h, int a, int d, int e, int g) : 
        name(n), health(h), attack(a), defense(d), experienceReward(e), goldReward(g) {}
};

class Game {
private:
    Character player;
    std::vector<Monster> monsters;
    std::vector<std::shared_ptr<Item>> items;
    std::mt19937 rng;
    std::map<std::string, std::function<void()>> commands;

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void pause(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    int getRandomNumber(int min, int max) {
        return std::uniform_int_distribution<int>{min, max}(rng);
    }

    void initializeMonsters() {
        monsters = {
            Monster("Gobelin", 20, 5, 2, 10, 5),
            Monster("Orc", 30, 7, 3, 15, 10),
            Monster("Troll", 50, 10, 5, 25, 20),
            Monster("Dragon", 100, 15, 8, 50, 50)
        };
    }

    void initializeItems() {
        items.push_back(std::make_shared<Item>("Épée en fer", 0, 5, 0));
        items.push_back(std::make_shared<Item>("Armure en cuir", 10, 0, 3));
        items.push_back(std::make_shared<Item>("Potion de vie", 20, 0, 0));
        items.push_back(std::make_shared<Item>("Amulette de force", 5, 3, 1));
    }

    void displayStats() {
        std::cout << "\n--- " << player.name << " ---" << std::endl;
        std::cout << "Niveau: " << player.level << " | XP: " << player.experience << " | Or: " << player.gold << std::endl;
        std::cout << "Santé: " << player.health << "/" << player.maxHealth << std::endl;
        std::cout << "Attaque: " << player.attack << " | Défense: " << player.defense << std::endl;
    }

    void battle() {
        Monster enemy = monsters[getRandomNumber(0, monsters.size() - 1)];
        std::cout << "\nVous rencontrez un " << enemy.name << " !" << std::endl;

        while (enemy.health > 0 && player.health > 0) {
            displayStats();
            std::cout << "\n" << enemy.name << " - Santé: " << enemy.health << std::endl;
            std::cout << "\n1. Attaquer" << std::endl;
            std::cout << "2. Fuir" << std::endl;
            
            int choice = getValidInput(1, 2);

            if (choice == 1) {
                int playerDamage = std::max(0, player.attack - enemy.defense);
                enemy.health -= playerDamage;
                std::cout << "Vous infligez " << playerDamage << " dégâts à " << enemy.name << "." << std::endl;

                if (enemy.health > 0) {
                    int enemyDamage = std::max(0, enemy.attack - player.defense);
                    player.health -= enemyDamage;
                    std::cout << enemy.name << " vous inflige " << enemyDamage << " dégâts." << std::endl;
                }
            } else if (choice == 2) {
                std::cout << "Vous fuyez le combat." << std::endl;
                return;
            }

            pause(1000);
            clearScreen();
        }

        if (player.health > 0) {
            std::cout << "Vous avez vaincu " << enemy.name << " !" << std::endl;
            player.experience += enemy.experienceReward;
            player.gold += enemy.goldReward;
            std::cout << "Vous gagnez " << enemy.experienceReward << " points d'expérience et " << enemy.goldReward << " pièces d'or." << std::endl;

            if (player.experience >= player.level * 20) {
                player.levelUp();
            }

            if (getRandomNumber(1, 100) <= 30) {
                player.addItem(items[getRandomNumber(0, items.size() - 1)]);
            }
        } else {
            std::cout << "Vous avez été vaincu par " << enemy.name << "." << std::endl;
        }
    }

    void rest() {
        int healAmount = player.maxHealth / 2;
        player.health = std::min(player.maxHealth, player.health + healAmount);
        std::cout << "Vous vous reposez et récupérez " << healAmount << " points de vie." << std::endl;
    }

    void shop() {
        std::cout << "\n--- Boutique ---" << std::endl;
        std::cout << "Or disponible : " << player.gold << std::endl;
        std::cout << "1. Potion de vie (20 or)" << std::endl;
        std::cout << "2. Épée en acier (50 or)" << std::endl;
        std::cout << "3. Armure en mailles (40 or)" << std::endl;
        std::cout << "4. Quitter la boutique" << std::endl;

        int choice = getValidInput(1, 4);

        switch (choice) {
            case 1:
                if (player.gold >= 20) {
                    player.gold -= 20;
                    player.addItem(std::make_shared<Item>("Potion de vie", 30, 0, 0));
                } else {
                    std::cout << "Vous n'avez pas assez d'or." << std::endl;
                }
                break;
            case 2:
                if (player.gold >= 50) {
                    player.gold -= 50;
                    player.addItem(std::make_shared<Item>("Épée en acier", 0, 8, 0));
                } else {
                    std::cout << "Vous n'avez pas assez d'or." << std::endl;
                }
                break;
            case 3:
                if (player.gold >= 40) {
                    player.gold -= 40;
                    player.addItem(std::make_shared<Item>("Armure en mailles", 15, 0, 5));
                } else {
                    std::cout << "Vous n'avez pas assez d'or." << std::endl;
                }
                break;
            case 4:
                std::cout << "Vous quittez la boutique." << std::endl;
                break;
        }
    }

    int getValidInput(int min, int max) {
        int choice;
        while (true) {
            std::cout << "Votre choix (" << min << "-" << max << "): ";
            if (std::cin >> choice && choice >= min && choice <= max) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return choice;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrée invalide. Veuillez réessayer." << std::endl;
        }
    }

    void initializeCommands() {
        commands["explorer"] = [this]() { battle(); };
        commands["repos"] = [this]() { rest(); };
        commands["stats"] = [this]() { displayStats(); };
        commands["inventaire"] = [this]() { player.displayInventory(); };
        commands["boutique"] = [this]() { shop(); };
        commands["aide"] = [this]() {
            std::cout << "\nCommandes disponibles :" << std::endl;
            for (const auto& cmd : commands) {
                std::cout << "- " << cmd.first << std::endl;
            }
        };
    }

public:
    Game() : player("Héros", 50, 10, 5), rng(std::random_device{}()) {
        initializeMonsters();
        initializeItems();
        initializeCommands();
    }

    void start() {
        std::cout << "Bienvenue dans l'Aventure Fantastique !" << std::endl;
        std::cout << "Quel est votre nom, brave aventurier ? ";
        std::getline(std::cin, player.name);

        std::cout << "\nTapez 'aide' pour voir la liste des commandes disponibles." << std::endl;

        while (player.health > 0) {
            std::cout << "\nQue souhaitez-vous faire ? ";
            std::string input;
            std::getline(std::cin, input);

            std::transform(input.begin(), input.end(), input.begin(), ::tolower);

            if (input == "quitter") {
                std::cout << "Merci d'avoir joué ! Au revoir." << std::endl;
                return;
            }

            auto it = commands.find(input);
            if (it != commands.end()) {
                it->second();
            } else {
                std::cout << "Commande inconnue. Tapez 'aide' pour voir la liste des commandes." << std::endl;
            }

            pause(1000);
        }

        std::cout << "Game Over. Votre aventure s'arrête ici." << std::endl;
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}
