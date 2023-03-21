# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.

import collections.abc
import copy
import os

import brave_node
import chromium_presubmit_overrides
import git_cl
import import_inline
import override_utils

USE_PYTHON3 = True
PRESUBMIT_VERSION = '2.0.0'

# pylint: disable=line-too-long,protected-access


# Adds support for chromium_presubmit_config.json5 and some helpers.
def CheckToModifyInputApi(input_api, _output_api):
    chromium_presubmit_overrides.modify_input_api(input_api)
    return []


# Check and fix formatting issues (supports --fix).
def CheckPatchFormatted(input_api, output_api):
    # Use git cl format to format supported files with Chromium formatters.
    git_cl_format_cmd = [
        '-C',
        input_api.change.RepositoryRoot(),
        'cl',
        'format',
        '--presubmit',
        '--python',
    ]

    # Make sure the passed --upstream branch is applied to git cl format.
    if input_api.change.UpstreamBranch():
        git_cl_format_cmd.extend(
            ['--upstream', input_api.change.UpstreamBranch()])

    # Do a dry run if --fix was not passed.
    if not input_api.PRESUBMIT_FIX:
        git_cl_format_cmd.append('--dry-run')

    # Pass a path where the current PRESUBMIT.py file is located.
    git_cl_format_cmd.append(input_api.PresubmitLocalPath())

    # Run git cl format and get return code.
    git_cl_format_code, _ = git_cl.RunGitWithCode(git_cl_format_cmd,
                                                  suppress_stderr=True)

    is_format_required = git_cl_format_code == 2

    if not is_format_required or input_api.PRESUBMIT_FIX:
        # Use Prettier to format other file types.
        files_to_check = (
            # Enable when files will be formatted.
            # r'.+\.js$',
            # r'.+\.ts$',
            # r'.+\.tsx$',
        )
        files_to_skip = input_api.DEFAULT_FILES_TO_SKIP

        file_filter = lambda f: input_api.FilterSourceFile(
            f, files_to_check=files_to_check, files_to_skip=files_to_skip)
        affected_files = input_api.AffectedFiles(file_filter=file_filter,
                                                 include_deletes=False)
        files_to_format = [f.AbsoluteLocalPath() for f in affected_files]

        node_args = [
            brave_node.PathInNodeModules('prettier', 'bin-prettier'),
            '--write' if input_api.PRESUBMIT_FIX else '--check',
        ]

        files_per_command = 25 if input_api.is_windows else 1000
        for i in range(0, len(files_to_format), files_per_command):
            args = node_args + files_to_format[i:i + files_per_command]
            try:
                brave_node.RunNode(args)
            except RuntimeError as err:
                if 'Forgot to run Prettier?' in str(err):
                    is_format_required = True
                    break
                # Raise on unexpected output. Could be node or prettier issues.
                raise

    if is_format_required:
        if input_api.PRESUBMIT_FIX:
            raise RuntimeError('--fix was passed, but format has failed')
        short_path = input_api.basename(input_api.change.RepositoryRoot())
        return [
            output_api.PresubmitError(
                f'The {short_path} directory requires source formatting. '
                'Please run: npm run presubmit -- --fix')
        ]
    return []


# Check and fix ESLint issues (supports --fix).
def CheckESLint(input_api, output_api):
    files_to_check = (
        r'.+\.js$',
        r'.+\.ts$',
        r'.+\.tsx$',
    )
    files_to_skip = input_api.DEFAULT_FILES_TO_SKIP

    file_filter = lambda f: input_api.FilterSourceFile(
        f, files_to_check=files_to_check, files_to_skip=files_to_skip)
    files_to_check = input_api.AffectedFiles(file_filter=file_filter,
                                             include_deletes=False)

    with import_inline.sys_path(
            input_api.os_path.join(input_api.PresubmitLocalPath(), '..',
                                   'tools')):
        # pylint: disable=import-error,import-outside-toplevel
        from web_dev_style import js_checker
        return js_checker.JSChecker(input_api,
                                    output_api).RunEsLintChecks(files_to_check)


def CheckWebDevStyle(input_api, output_api):
    with import_inline.sys_path(
            input_api.os_path.join(input_api.PresubmitLocalPath(), '..',
                                   'tools')):
        # pylint: disable=import-error,import-outside-toplevel
        from web_dev_style import presubmit_support, js_checker
        # Disable RunEsLintChecks, it's run separately in CheckESLint.
        with override_utils.override_scope_function(
                js_checker.JSChecker,
                chromium_presubmit_overrides.noop_check,
                name='RunEsLintChecks'):
            return presubmit_support.CheckStyle(input_api, output_api)


def CheckChangeLintsClean(input_api, output_api):
    return input_api.canned_checks.CheckChangeLintsClean(input_api,
                                                         output_api,
                                                         lint_filters=[])


def CheckPylint(input_api, output_api):
    extra_paths_list = os.environ['PYTHONPATH'].split(os.pathsep)
    return input_api.canned_checks.RunPylint(input_api,
                                             output_api,
                                             pylintrc=input_api.os_path.join(
                                                 input_api.PresubmitLocalPath(),
                                                 '.pylintrc'),
                                             extra_paths_list=extra_paths_list)


def CheckLicense(input_api, output_api):
    """Verifies the Brave license header."""

    files_to_check = input_api.DEFAULT_FILES_TO_CHECK + (r'.+\.gni?$', )
    files_to_skip = input_api.DEFAULT_FILES_TO_SKIP + (
        r"\.storybook/",
        r"ios/browser/api/ledger/legacy_database/core_data_models/",
        r'win_build_output/',
    )

    current_year = int(input_api.time.strftime('%Y'))
    allowed_years = (str(s) for s in reversed(range(2015, current_year + 1)))
    years_re = '(' + '|'.join(allowed_years) + ')'

    # License regexp to match in NEW and MOVED files, it doesn't allow variance.
    # Note: presubmit machinery cannot distinguish between NEW and MOVED files,
    # that's why we cannot force this regexp to have a precise year, also
    # uplifts may fail during year change period, so the year check is relaxed.
    new_file_license_re = input_api.re.compile((
        r'.*? Copyright \(c\) %(year)s The Brave Authors\. All rights reserved\.\n'
        r'.*? This Source Code Form is subject to the terms of the Mozilla Public\n'
        r'.*? License, v\. 2\.0\. If a copy of the MPL was not distributed with this file,\n'
        r'.*? You can obtain one at https://mozilla.org/MPL/2\.0/\.(?: \*/)?\n')
                                               % {'year': years_re},
                                               input_api.re.MULTILINE)

    # License regexp to match in EXISTING files, it allows some variance.
    existing_file_license_re = input_api.re.compile((
        r'.*? Copyright \(c\) %(year)s The Brave Authors\. All rights reserved\.\n'
        r'.*? This Source Code Form is subject to the terms of the Mozilla Public\n'
        r'.*? License, v\. 2\.0\. If a copy of the MPL was not distributed with this(\n.*?)? file,\n?'
        r'.*? (y|Y)ou can obtain one at https?://mozilla.org/MPL/2\.0/\.(?: \*/)?\n'
    ) % {'year': years_re}, input_api.re.MULTILINE)

    # License template for new files. Includes current year.
    expected_license_template = (
        '%(comment)s Copyright (c) %(year)s The Brave Authors. All rights reserved.\n'
        '%(comment)s This Source Code Form is subject to the terms of the Mozilla Public\n'
        '%(comment)s License, v. 2.0. If a copy of the MPL was not distributed with this file,\n'
        '%(comment)s You can obtain one at https://mozilla.org/MPL/2.0/.\n') % {
            'comment': '#',
            'year': current_year,
        }

    bad_new_files = []
    bad_files = []
    sources = lambda affected_file: input_api.FilterSourceFile(
        affected_file,
        files_to_check=files_to_check,
        files_to_skip=files_to_skip)
    for f in input_api.AffectedSourceFiles(sources):
        contents = input_api.ReadFile(f, 'r')[:1000].replace('\r\n', '\n')
        if not contents:
            continue
        if f.Action() == 'A':  # 'A' means "Added", also includes moved files.
            if not new_file_license_re.search(contents):
                bad_new_files.append(f.LocalPath())
        else:
            if not existing_file_license_re.search(contents):
                bad_files.append(f.LocalPath())

    splitted_expected_license_template = expected_license_template.replace(
        "# ", "").split('\n')
    multiline_comment_expected_license = (
        f'/* {splitted_expected_license_template[0]}\n'
        f' * {splitted_expected_license_template[1]}\n'
        f' * {splitted_expected_license_template[2]}\n'
        f' * {splitted_expected_license_template[3]} */\n')
    assert new_file_license_re.search(expected_license_template)
    assert existing_file_license_re.search(expected_license_template)
    assert new_file_license_re.search(multiline_comment_expected_license)
    assert existing_file_license_re.search(multiline_comment_expected_license)

    # Show this to simplify copy-paste when an invalid license is found.
    expected_licenses = (f'{expected_license_template.replace("#", "//")}\n'
                         f'{multiline_comment_expected_license}\n'
                         f'{expected_license_template}')

    result = []
    if bad_new_files:
        expected_license_message = (
            f'Expected one of license headers in new files:\n'
            f'{expected_licenses}')
        result.append(
            output_api.PresubmitError(expected_license_message,
                                      items=bad_new_files))
    if bad_files:
        expected_license_message = (
            f'Expected one of license headers in existing files:\n'
            f'{expected_licenses.replace(f"{current_year}", "<year>")}')
        result.append(
            output_api.PresubmitPromptWarning(expected_license_message,
                                              items=bad_files))
    return result


# DON'T ADD NEW BRAVE CHECKS AFTER THIS LINE.
#
# This call inlines Chromium checks into current scope from src/PRESUBMIT.py. We
# do this to have the right order of checks, so all `--fix`-aware checks are
# executed first.
chromium_presubmit_overrides.inline_presubmit_from_src('PRESUBMIT.py',
                                                       globals(), locals())


# Extend BanRule exclude lists with Brave-specific paths.
def ApplyBanRuleExcludes():
    # Collect all _BANNED_* variables declared in //PRESUBMIT.py.
    ban_rule_lists = [
        value for name, value in globals().items()
        if name.startswith('_BANNED_')
        and isinstance(value, collections.abc.Sequence) and len(value) > 0
        and isinstance(value[0], BanRule)  # pylint: disable=undefined-variable
    ]

    # Get additional excluded paths from the config.
    ban_rule_excluded_paths = chromium_presubmit_overrides.config.get(
        'ban_rule_excluded_paths')

    # Add excluded paths to BanRule instances.
    all_patterns = {*ban_rule_excluded_paths.keys()}
    used_patterns = set()
    for ban_rule_list in ban_rule_lists:
        for ban_rule in ban_rule_list:
            excluded_paths = ban_rule_excluded_paths.get(ban_rule.pattern)
            if not excluded_paths:
                continue

            used_patterns.add(ban_rule.pattern)
            if ban_rule.excluded_paths is None:
                ban_rule.excluded_paths = (*excluded_paths, )
            else:
                ban_rule.excluded_paths += (*excluded_paths, )

    # Fail if some pattern was not used.
    unused_patterns = all_patterns - used_patterns
    if unused_patterns:
        raise RuntimeError(f'ERROR: Unused ban_rule_excluded_paths patterns: '
                           f'{unused_patterns}')


ApplyBanRuleExcludes()


@chromium_presubmit_overrides.override_check(globals())
def CheckForIncludeGuards(original_check, input_api, output_api, **kwargs):
    # Add 'brave/' prefix for header guard checks to properly validate guards.
    def AffectedSourceFiles(self, original_method, source_file):
        def PrependBrave(affected_file):
            affected_file = copy.copy(affected_file)
            affected_file._path = f'brave/{affected_file._path}'
            return affected_file

        return [
            PrependBrave(f)
            for f in filter(self.FilterSourceFile, original_method(source_file))
        ]

    with override_utils.override_scope_function(input_api, AffectedSourceFiles):
        return original_check(input_api, output_api, **kwargs)


# Use BanRule.excluded_paths in all BanRule-like checks.
@override_utils.override_function(globals())
def _GetMessageForMatchingType(orig, input_api, f, line_num, line, ban_rule):
    def IsExcludedFile(affected_file, excluded_paths):
        if not excluded_paths:
            return False

        local_path = affected_file.LocalPath()
        # Consistently use / as path separator to simplify the writing of regex
        # expressions.
        local_path = local_path.replace(input_api.os_path.sep, '/')
        for item in excluded_paths:
            if input_api.re.match(item, local_path):
                return True
        return False

    if IsExcludedFile(f, ban_rule.excluded_paths):
        return []

    return orig(input_api, f, line_num, line, ban_rule)


@override_utils.override_function(globals())
def _ChangeHasSecurityReviewer(*_):
    # We don't have Gerrit API available to check for reviewers.
    return False


# DON'T ADD NEW CHECKS HERE, ADD THEM BEFORE FIRST inline_presubmit_from_src().
