#include "minishell.h"

static bool     is_quoted(char *delim)
{
    t_ulong len;

    len = ms_strlen(delim);
    if (len < 2)
        return (false);
    if ((delim[0] == '\'' && delim[len - 1] == '\'')
        || (delim[0] == '"' && delim[len - 1] == '"'))
        return (true);
    return (false);
}

static char     *strip_quotes(char *delim)
{
    t_ulong len;

    len = ms_strlen(delim);
    if (is_quoted(delim))
        return (ms_substr(delim, 1, len - 2));
    return (ms_strdup(delim));
}

static void     write_heredoc_lines(int fd, char *delim, bool exp_env)
{
    char        *line;
    t_shell     *shell;

    shell = get_shell();
    while (true)
    {
        line = readline("> ");
        if (!line || ms_strequals(line, delim))
            return (free(line));
        if (exp_env)
            line = expand(shell, line);
        write(fd, line, ms_strlen(line));
        write(fd, "\n", 1);
        free(line);
    }
}

char    *generate_tempfile_path(void)
{
    int         tmp_num;
    int         fd;
    char        *tmp_num_str;
    char        *path;

    tmp_num = 0;
    while (true)
    {
        tmp_num_str = ms_itoa(tmp_num);
        path = ms_strjoin(HD_TEMP_PATH, tmp_num_str, '\0');
        free(tmp_num_str);
        fd = open(path, O_CREAT | O_EXCL | O_RDWR, PERM_URW_GR_OR);
        if (fd != -1)
            return (close(fd), path);
        free(path);
        tmp_num++;
    }
}

static int      hd_child_status(int status, char *path)
{
    t_shell *shell;

    shell = get_shell();
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
    {
        shell->last_exit_code = 130;
        unlink(path);
        free(path);
        rl_replace_line("", 0);
        rl_on_new_line();
        rl_redisplay();
        return (1);
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        shell->last_exit_code = WEXITSTATUS(status);
        unlink(path);
        free(path);
        return (1);
    }
    return (0);
}

static int      wait_heredoc_child(pid_t pid, char *path)
{
    int status;

    waitpid(pid, &status, 0);
    get_shell()->heredoc_pid = 0;
    return (hd_child_status(status, path));
}

static pid_t    fork_hd_process(char *delim, char *path, int fd, bool exp_env)
{
    pid_t       pid;

    pid = fork();
    if (pid == -1)
    {
        close(fd);
        free(path);
        return (-1);
    }
    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_IGN);
        write_heredoc_lines(fd, delim, exp_env);
        close(fd);
        exit(0);
    }
    return (pid);
}

char    *create_heredoc(char *delimiter)
{
    char        *path, *clean;
    int         fd;
    pid_t       pid;
    bool        exp_env;
    t_shell     *shell;

    shell = get_shell();
    exp_env = !is_quoted(delimiter);
    clean = strip_quotes(delimiter);
    path = generate_tempfile_path();
    fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1)
        return (free(path), free(clean), NULL);
    pid = fork_hd_process(clean, path, fd, exp_env);
    free(clean);
    if (pid == -1)
        return (NULL);
    shell->heredoc_pid = pid;
    if (wait_heredoc_child(pid, path))
        return (NULL);
    close(fd);
    return (path);
}

