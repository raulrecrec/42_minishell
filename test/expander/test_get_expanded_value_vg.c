/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_get_expanded_value_vg.c                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniema3 <daniema3@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 12:40:43 by daniema3          #+#    #+#             */
/*   Updated: 2025/07/08 13:25:08 by daniema3         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include "ms_assertions.h"

int	main(int argc, char **argv, char **env)
{
	t_shell	*shell;

	(void) argc;
	(void) argv;
	shell = init_shell(env);
	env_export(shell, "TEST_VAR=12345");
	env_export(shell, "LONG_KEY_WITH_STR_VALUE=Hello");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $TEST_VAR ...", 6), "12345");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $TEST_VAR", 6), "12345");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $?", 6), "0");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $DOESNT_EXIST", 6), "");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $LONG_KEY_WITH_STR_VALUE", 6), "Hello");
	shell->last_exit_code = 42;
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $?", 6), "42");
	FREE_ASSERT_STR_EQUALS(get_expanded_value(shell, "echo $", 6), "$");
	ms_exit(EXEC_OK, NULL);
}
