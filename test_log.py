from datetime import datetime

duration_to_beats = {
    "w": 4.0,  # ronde
    "h": 2.0,  # blanche
    "q": 1.0,  # noire
    "e": 0.5,  # croche
    "s": 0.25  # double croche
}

# Lecture et traitement
with open("../log.txt", "r") as f:
    lines = f.readlines()

measures = []
current_measure = {
    "start_time": None,
    "end_time": None,
    "events": [],
    "total_beats": 0
}

for line in lines:
    line = line.strip()
    if not line:
        continue

    # Découper la ligne
    if " - " not in line:
        continue

    timestamp_str, event = line.split(" - ", 1)
    timestamp = datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M:%S.%f")

    if "-- Debut de la partition" in event:
        continue
    elif "--- Barre de mesure" in event:
        current_measure["end_time"] = timestamp
        measures.append(current_measure)
        current_measure = {
            "start_time": timestamp,
            "end_time": None,
            "events": [],
            "total_beats": 0
        }
    elif "-- Fin de la partition" in event:
        current_measure["end_time"] = timestamp
        measures.append(current_measure)
    else:
        # Extrait la durée si possible
        for dur_symbol in duration_to_beats:
            if f"durée: {dur_symbol}" in event:
                current_measure["total_beats"] += duration_to_beats[dur_symbol]
                break
        current_measure["events"].append((timestamp, event))
        if not current_measure["start_time"]:
            current_measure["start_time"] = timestamp

# Écriture dans test_log.txt
with open("test_log.txt", "w", encoding="utf-8") as out:
    for i, m in enumerate(measures):
        duration_ms = (m["end_time"] - m["start_time"]).total_seconds() * 1000
        out.write(f"Mesure {i+1}:\n")
        out.write(f"  - Temps écoulé: {duration_ms:.1f} ms\n")
        out.write(f"  - Total beats: {m['total_beats']}\n")
        out.write(f"  - Événements: {len(m['events'])}\n\n")
