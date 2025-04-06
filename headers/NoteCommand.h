#ifndef NOTECOMMAND_H
#define NOTECOMMAND_H

#include <string>
#include <memory>
#include <lomse_interactor.h>  // Pour la d�finition de lomse::Interactor

class NoteCommand {
public:
    // Constructeur : noteSymbol doit �tre une cha�ne comme "c4" ou "d#4".
    NoteCommand(const std::string& noteSymbol, std::shared_ptr<lomse::Interactor> spInteractor);

    // Destructeur
    ~NoteCommand();

    // M�thode d'ex�cution de la commande d'insertion de note.
    void execute();

private:
    std::string m_noteSymbol;
    std::shared_ptr<lomse::Interactor> m_spInteractor;
};

#endif // NOTECOMMAND_H
