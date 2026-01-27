#include <iostream>
#include <string>
#include <list>
#include <map>
#include <functional>
#include <algorithm>

using namespace std;

using Command = function<void(const list<string>&)>;

class Entity {
private:
    string nombre;
    int vida;
    int energia;

public:
    Entity(string _nombre, int _vida) : nombre(_nombre), vida(_vida), energia(100) {}

    void curar(int n) { vida += n; }
    void danar(int n) { vida -= n; }
    void usar_energia(int n) { energia -= n; }
    void resetear() { vida = 100; energia = 100; }

    string get_status() const {
        return nombre + " (V:" + to_string(vida) + " E:" + to_string(energia) + ")";
    }
};

class CommandCenter {
private:
    Entity& entity;
    map<string, Command> comandos;
    list<string> historial;
    map<string, list<pair<string, list<string>>>> macros;

public:
    CommandCenter(Entity& _entity) : entity(_entity) {}

    void registerCommand(string nombre, Command cmd) {
        comandos[nombre] = cmd;
    }

    void execute(string nombre, const list<string>& args) {
        map<string, Command>::iterator it = comandos.find(nombre);

        if (it != comandos.end()) {
            string antes = entity.get_status();
            it->second(args);
            string despues = entity.get_status();

            historial.push_back("Comando: " + nombre + " | Antes: " + antes + " -> Despues: " + despues);
        } else {
            cout << "Error: El comando '" << nombre << "' no existe." << endl;
        }
    }

    void registerMacro(const string& name, const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it_macro = macros.find(name);
        if (it_macro != macros.end()) {
            list<pair<string, list<string>>>::iterator it_step;
            for (it_step = it_macro->second.begin(); it_step != it_macro->second.end(); ++it_step) {
                execute(it_step->first, it_step->second);
            }
        } else {
            cout << "Error: La macro '" << name << "' no existe." << endl;
        }
    }

    void mostrar_historial() {
        cout << "\n--- HISTORIAL DE COMANDOS ---" << endl;
        for (list<string>::iterator it = historial.begin(); it != historial.end(); ++it)
            cout << "* " << *it << endl;
    }
};

void cmd_status(Entity& e, const list<string>& args) {
    cout << "[STATUS] Entidad actual: " << e.get_status() << endl;
}

class DamageCommand {
private:
    Entity& entity;
    int contador_usos;
public:
    DamageCommand(Entity& _e) : entity(_e), contador_usos(0) {}
    void operator()(const list<string>& args) {
        if (args.empty()) return;
        entity.danar(stoi(args.front()));
        contador_usos++;
        cout << "(Dano total aplicado " << contador_usos << " veces)" << endl;
    }
};

int main() {
    Entity mi_entidad("Sebastian_UTEC", 100);
    CommandCenter center(mi_entidad);

    center.registerCommand("heal", [&mi_entidad](const list<string>& args) {
        if (!args.empty()) mi_entidad.curar(stoi(args.front()));
    });

    center.registerCommand("status", [&mi_entidad](const list<string>& args) {
        cmd_status(mi_entidad, args);
    });

    center.registerCommand("damage", DamageCommand(mi_entidad));

    center.registerCommand("reset", [&mi_entidad](const list<string>& args) {
        mi_entidad.resetear();
    });

    center.registerMacro("recovery", { {"heal", {"50"}}, {"status", {}} });
    center.registerMacro("triple_damage", { {"damage", {"10"}}, {"damage", {"10"}}, {"damage", {"10"}} });
    center.registerMacro("hard_reset", { {"reset", {}}, {"status", {}} });

    center.execute("damage", {"30"});
    center.executeMacro("recovery");
    center.executeMacro("triple_damage");

    center.mostrar_historial();

    return 0;
}