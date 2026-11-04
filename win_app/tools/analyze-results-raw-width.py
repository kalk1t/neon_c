from pathlib import Path
s=(Path(__file__).parent/'analyze-results-shaping.py').read_text().replace('8776:9.90625','8776:9.90234375').replace('results-shaping-analysis.json','results-shaping-raw-fallback-analysis.json')
exec(compile(s,'source-font-raw-advance-analysis','exec'),{'__file__':__file__})
