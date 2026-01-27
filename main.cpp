#include <iostream>
#include <string>
#include <list>
#include <map>
#include <functional>
#include <algorithm>
#include <stdexcept>

using namespace std;

using Command = function<void(const list<string>&)>;

class Entity {
private:
    string nombre;
    int vida;
    int energia;

public:
    Entity(string _nombre, int _vida) : nombre(_nombre), vida(_vida), energia(100) {}

    void curar(int n) {
        if (n < 0) throw invalid_argument("Valor negativo no permitido");
        vida = min(100, vida + n);
    }
    void danar(int n) {
        if (n < 0) throw invalid_argument("Valor negativo no permitido");
        vida = max(0, vida - n);
    }
    void usar_energia(int n) {
        if (n < 0) throw invalid_argument("Valor negativo no permitido");
        energia = max(0, energia - n);
    }
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

    void removeCommand(string nombre) {
        if (comandos.erase(nombre)) {
            cout << "[SISTEMA] Comando '" << nombre << "' eliminado." << endl;
        }
    }

    void execute(string nombre, const list<string>& args) {
        auto it = comandos.find(nombre);
        if (it == comandos.end()) {
            string error = "ERROR: Comando '" + nombre + "' inexistente.";
            historial.push_back(error);
            throw runtime_error(error);
        }

        string antes = entity.get_status();
        try {
            it->second(args);
            string despues = entity.get_status();
            historial.push_back("Comando: " + nombre + " | " + antes + " -> " + despues);
        } catch (const exception& e) {
            historial.push_back("Fallo en '" + nombre + "': " + string(e.what()));
            throw;
        }
    }

    void registerMacro(const string& name, const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it_macro = macros.find(name);
        if (it_macro == macros.end()) {
            cout << "Error: Macro '" << name << "' inexistente." << endl;
            return;
        }

        cout << "\n> Ejecutando Macro: " << name << endl;
        try {
            for (auto& step : it_macro->second) {
                execute(step.first, step.second);
            }
        } catch (...) {
            cout << "!! Macro interrumpida por error de validacion." << endl;
        }
    }

    void mostrar_historial() {
        cout << "\n--- HISTORIAL DE IMPACTO ---" << endl;
        for (auto it = historial.begin(); it != historial.end(); ++it)
            cout << "* " << *it << endl;
    }
};

void f_curar(Entity& e, const list<string>& args) {
    if (args.empty()) throw runtime_error("Sin argumentos");
    e.curar(stoi(args.front()));
}
void f_status(Entity& e, const list<string>& args) { cout << "[INFO] " << e.get_status() << endl; }
void f_reset(Entity& e, const list<string>& args) { e.resetear(); }

struct DamageFunctor {
    Entity& entity;
    void operator()(const list<string>& args) {
        if (args.empty()) throw runtime_error("Sin valor de dano");
        entity.danar(stoi(args.front()));
    }
};
struct EnergyFunctor {
    Entity& entity;
    void operator()(const list<string>& args) {
        if (args.empty()) throw runtime_error("Sin valor de energia");
        entity.usar_energia(stoi(args.front()));
    }
};
struct SysLog {
    void operator()(const list<string>& args) { cout << "[LOG] Operacion realizada." << endl; }
};

int main() {
    Entity sebastian("Sebastian_UTEC", 100);
    CommandCenter center(sebastian);

    center.registerCommand("heal", bind(f_curar, ref(sebastian), placeholders::_1));
    center.registerCommand("status", bind(f_status, ref(sebastian), placeholders::_1));
    center.registerCommand("reset", bind(f_reset, ref(sebastian), placeholders::_1));

    center.registerCommand("damage", DamageFunctor{sebastian});
    center.registerCommand("burn", EnergyFunctor{sebastian});
    center.registerCommand("log", SysLog{});

    center.registerCommand("full_hp", [&](const list<string>& a){ sebastian.curar(100); });
    center.registerCommand("poison", [&](const list<string>& a){ sebastian.danar(10); });
    center.registerCommand("msg", [&](const list<string>& a){ cout << "Sistema activo" << endl; });

    cout << "--- PRUEBAS DE FUNCIONAMIENTO ---" << endl;
    center.execute("damage", {"15"});

    try {
        center.execute("invalid_cmd", {});
    } catch(const exception& e) { cout << "Validacion: " << e.what() << endl; }

    try {
        center.execute("heal", {"-10"});
    } catch(const exception& e) { cout << "Validacion: " << e.what() << endl; }

    center.removeCommand("burn");

    center.registerMacro("recovery_seq", { {"heal", {"20"}}, {"status", {}}, {"log", {}} });
    center.executeMacro("recovery_seq");

    center.mostrar_historial();

    return 0;
}