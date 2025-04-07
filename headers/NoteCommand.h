#ifndef NOTECOMMAND_H
#define NOTECOMMAND_H

#include <string>
#include <memory>
#include <lomse_interactor.h>  // Pour la définition de lomse::Interactor

class NoteCommand {
public:
    // Constructeur : noteSymbol doit être une chaîne comme "c4" ou "d#4".
    NoteCommand(const std::string& typeSymbol, const std::string& noteSymbol, const std::string& dureeSymbol, std::shared_ptr<lomse::Interactor> spInteractor);

    // Destructeur
    ~NoteCommand();

    // Méthode d'exécution de la commande d'insertion de note.
    void execute();

private:
    std::string m_typeSymbol;
    std::string m_noteSymbol;
    std::string m_dureeSymbol;
    
    std::shared_ptr<lomse::Interactor> m_spInteractor;
};

#endif // NOTECOMMAND_H
