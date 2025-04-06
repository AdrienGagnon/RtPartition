#include "NoteCommand.h"
#include <sstream>
#include <lomse_command.h>   // Pour CmdInsertStaffObj
#include <lomse_ldp_exporter.h> // Si besoin pour convertir le type de clef, etc.

using namespace lomse;

NoteCommand::NoteCommand(const std::string& noteSymbol, std::shared_ptr<Interactor> spInteractor)
    : m_noteSymbol(noteSymbol)
    , m_spInteractor(spInteractor)
{
}

NoteCommand::~NoteCommand() {
    // Rien de particulier � faire si la commande est g�r�e par l'interactor
}

void NoteCommand::execute() {
    if (m_spInteractor) {
        // Construire la cha�ne LDP pour la note.
        // Ici, "(n c4 q)" ins�re une note avec le symbole fourni (exemple "c4")
        // et "q" indique une noire (quarter note). Vous pouvez adapter la dur�e au besoin.
        std::stringstream src;
        src << "(n " << m_noteSymbol << " q)";
        std::string ldpSource = src.str();

        // Nom de la commande pour les logs ou l'undo
        std::string cmdName = "Insert note " + m_noteSymbol;

        // Cr�er la commande CmdInsertStaffObj.
        // Le constructeur prend le LDP source et le nom de la commande.
        CmdInsertStaffObj* pCmd = new CmdInsertStaffObj(ldpSource, cmdName);

        // Ex�cuter la commande via l'interactor.
        m_spInteractor->exec_command(pCmd);
    }
}
