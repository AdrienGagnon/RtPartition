#include "NoteCommand.h"
#include <sstream>
#include <qdebug.h>
#include <lomse_command.h>   // Pour CmdInsertStaffObj
#include <lomse_ldp_exporter.h> // Si besoin pour convertir le type de clef, etc.

using namespace lomse;

NoteCommand::NoteCommand(const std::string& typeSymbol, const std::string& noteSymbol, const std::string& dureeSymbol, std::shared_ptr<Interactor> spInteractor)
    : m_typeSymbol(typeSymbol)
    , m_noteSymbol(noteSymbol)
    , m_dureeSymbol(dureeSymbol)
    , m_spInteractor(spInteractor)
{}

NoteCommand::~NoteCommand() {
    // Rien de particulier à faire si la commande est gérée par l'interactor
}

void NoteCommand::execute() {
    if (m_spInteractor) {
        // Construire la chaîne LDP pour la note.
        // Ici, "(n c4 q)" insère une note avec le symbole fourni (exemple "c4")
        // et "q" indique une noire (quarter note). Vous pouvez adapter la durée au besoin.
        std::stringstream src;
        if (m_noteSymbol == "bar") {
            src << "(barline)";
        }
        else if (m_noteSymbol == "end") {
            src << "(barline end)";
        }
        else if (m_noteSymbol == "r") {
            src << "(" << m_typeSymbol << " " << m_dureeSymbol << ")";
        }
        else {
            src << "(" << m_typeSymbol << " " << m_noteSymbol << " " << m_dureeSymbol << ")";
        }
        std::string ldpSource = src.str();

        // Nom de la commande pour les logs ou l'undo
        std::string cmdName = "Insert note " + m_noteSymbol;

        // Créer la commande CmdInsertStaffObj.
        // Le constructeur prend le LDP source et le nom de la commande.
        CmdInsertStaffObj* pCmd = new CmdInsertStaffObj(ldpSource, cmdName);

        // Exécuter la commande via l'interactor.
        m_spInteractor->exec_command(pCmd);
    }
}
