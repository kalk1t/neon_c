"""Validate source layout geometry and authored placeholder clearance only."""
import argparse
import json
from pathlib import Path
from route_tools import read_json, validate_layout


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--route",required=True,type=Path)
    parser.add_argument("--placements",type=Path)
    parser.add_argument("--report",type=Path,help="Optional new report file; refuses to overwrite")
    args=parser.parse_args()
    route=read_json(args.route)
    placements=read_json(args.placements) if args.placements else None
    report=validate_layout(route,placements)
    text=json.dumps(report,indent=2)
    print(text)
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        with args.report.open("x",encoding="utf-8") as handle:
            handle.write(text+"\n")
    raise SystemExit(0 if report["ok"] else 1)


if __name__=="__main__": main()
