#!/usr/bin/env python3
"""Merge existing City Life/PvP Life settings into the unified configuration.

Only explicitly supplied files are read. Input files are never changed. The
output keeps the template's sections, existing raw option values, custom Life
options, and source comments that are absent from the template.
"""

import argparse
import os
from pathlib import Path
import sys
import tempfile


NAMESPACES = ("CityLife.", "PvPLife.", "FishingLife.")


def migrate_fishing_options(options):
    """Move former Custom fishing values; explicit Fishing Life values win."""
    result = dict(options)
    legacy = "CityLife.Custom.Fishing."
    had_fishing = any(key.startswith(legacy) for key in result)
    for key in list(result):
        if key.startswith(legacy):
            result.setdefault("FishingLife." + key[len(legacy):], result.pop(key))
    if had_fishing:
        for suffix in ("TimeScaling", "ResidenceSeconds", "Chat.Range",
                       "CreateMissing.Enable", "CreateMissing.CreateIntervalSeconds"):
            shared = "CityLife.Custom." + suffix
            if shared in result:
                result.setdefault("FishingLife." + suffix, result[shared])
    return result


def read_config(path, *, template=False):
    """Read the core's first-assignment-wins format without interpreting values."""
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    options = {}
    comments = []
    duplicates = 0

    for line_number, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith("#"):
            comments.append(stripped)
            continue
        if stripped.startswith("["):
            section = stripped.split("#", 1)[0].strip()
            if section != "[worldserver]":
                raise ValueError(f"{path}:{line_number}: expected a [worldserver] section.")
            continue
        if "=" not in stripped:
            raise ValueError(f"{path}:{line_number}: expected an option assignment or # comment.")

        key, value = stripped.split("=", 1)
        key = key.strip()
        value = value.strip()
        if not key.startswith(NAMESPACES) or any(character.isspace() for character in key):
            raise ValueError(f"{path}:{line_number}: only CityLife.*, PvPLife.* and FishingLife.* options are accepted.")
        if key in NAMESPACES:
            raise ValueError(f"{path}:{line_number}: an option name is incomplete.")
        if key in options:
            if template:
                raise ValueError(f"{path}:{line_number}: the template contains a duplicate option.")
            duplicates += 1
            continue
        options[key] = value

    if not options:
        raise ValueError(f"{path}: no Life option assignments were found.")
    return lines, options, comments, duplicates


def render_config(template_lines, template_options, source_options, source_comments):
    result = []
    section_written = False
    for line in template_lines:
        stripped = line.strip()
        if stripped.startswith("["):
            if not section_written:
                result.append("[worldserver]")
                section_written = True
        elif stripped and not stripped.startswith("#") and "=" in stripped:
            key = stripped.split("=", 1)[0].strip()
            value = source_options.get(key, template_options[key])
            result.append(f"{key} = {value}")
        else:
            result.append(line.rstrip())
    if not section_written:
        result.insert(0, "[worldserver]")

    custom_count = 0
    for namespace, title in (("CityLife.", "City Life"), ("PvPLife.", "PvP Life"), ("FishingLife.", "Fishing Life")):
        custom = [(key, value) for key, value in source_options.items()
                  if key.startswith(namespace) and key not in template_options]
        if custom:
            result.extend(("", f"# Preserved custom {title} options"))
            result.extend(f"{key} = {value}" for key, value in custom)
            custom_count += len(custom)

    known_comments = {line.strip() for line in template_lines if line.strip().startswith("#")}
    notes = []
    for comment in source_comments:
        if comment not in known_comments:
            notes.append(comment)
            known_comments.add(comment)
    if notes:
        result.extend(("", "# Preserved notes from the previous configuration files", *notes))

    return "\n".join(result).rstrip() + "\n", custom_count


def validate_output(output, sources, *, force=False):
    """Protect all input files, including aliases through symlinks/hard links."""
    if output.is_symlink():
        raise ValueError("The output must not be a symbolic link.")
    for directory in (output.parent, *output.parent.parents):
        is_junction = getattr(directory, "is_junction", lambda: False)
        if directory.is_symlink() or is_junction():
            raise ValueError("The output parent path must not contain symbolic links or junctions.")
    parent = output.parent.resolve(strict=True)
    if not parent.is_dir():
        raise ValueError("The output parent must be an existing directory.")
    resolved_output = parent / output.name
    for source in sources:
        if resolved_output == source or (output.exists() and os.path.samefile(output, source)):
            raise ValueError("The output must be different from every input and the template, even with --force.")
    if output.exists():
        if not output.is_file():
            raise ValueError("The output must be a regular file.")
        if not force:
            raise ValueError("The output already exists; choose another path or use --force.")
    return resolved_output


def write_atomic(output, contents, sources, *, force=False):
    output = validate_output(output, sources, force=force)
    temporary_path = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", newline="\n", dir=output.parent,
                                         prefix=f".{output.name}.", suffix=".tmp", delete=False) as temporary:
            temporary_path = Path(temporary.name)
            temporary.write(contents)
            temporary.flush()
            os.fsync(temporary.fileno())
        validate_output(output, sources, force=force)
        os.replace(temporary_path, output)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--city-config", type=Path, help="Existing personal City Life .conf file.")
    parser.add_argument("--pvp-config", type=Path, help="Existing personal PvP Life .conf file (takes precedence).")
    parser.add_argument("--template", type=Path,
                        default=Path(__file__).resolve().parents[1] / "conf" / "mod_playerbots_city_life.conf.dist",
                        help="Unified .conf.dist template; defaults to this module's template.")
    parser.add_argument("--output", required=True, type=Path, help="Separate output file; its parent must already exist.")
    parser.add_argument("--force", action="store_true", help="Replace an existing output, never an input or template.")
    args = parser.parse_args(argv)
    if args.city_config is None and args.pvp_config is None:
        parser.error("at least one of --city-config and --pvp-config is required")

    try:
        template_path = args.template.resolve(strict=True)
        source_paths = [path.resolve(strict=True) for path in (args.city_config, args.pvp_config) if path is not None]
        protected_paths = [template_path, *source_paths]
        validate_output(args.output, protected_paths, force=args.force)
        template_lines, template_options, _, _ = read_config(template_path, template=True)
        source_options = {}
        source_comments = []
        for source in source_paths:
            _, options, comments, duplicates = read_config(source)
            source_options.update(options)
            source_comments.extend(comments)
            if duplicates:
                print(f"Warning: {source}: ignored {duplicates} duplicate assignments; first occurrence wins.",
                      file=sys.stderr)
        source_options = migrate_fishing_options(source_options)
        contents, custom_count = render_config(template_lines, template_options, source_options, source_comments)
        write_atomic(args.output, contents, protected_paths, force=args.force)
        print(f"Wrote {args.output.resolve()}: {len(template_options) + custom_count} options, "
              f"{custom_count} preserved custom options.")
        return 0
    except (OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
